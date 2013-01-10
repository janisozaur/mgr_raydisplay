#include "raydisplaywindow.h"
#include "ui_raydisplaywindow.h"
#include "RayDisplayScene.h"

#include <QTimer>
#include <QMessageBox>

#include <QDebug>

RayDisplayWindow::RayDisplayWindow(QWidget *parent) :
    QMainWindow(parent),
	ui(new Ui::RayDisplayWindow),
	mCurrentSender(0)
{
    ui->setupUi(this);
	mRDS = new RayDisplayScene(this);
	mDemoRayTimer = new QTimer(this);
	ui->modelDisplay->setScene(mRDS);
    qDebug() << mRDS->sceneRect();
    qDebug() << ui->modelDisplay->rect();
    //mRDS->setSceneRect(ui->modelDisplay->rect());
	qDebug() << mRDS->sceneRect();
	connect(ui->clearObstaclePushButton, SIGNAL(clicked()), mRDS, SLOT(clearObstacle()));
	connect(mDemoRayTimer, SIGNAL(timeout()), this, SLOT(senderTimeout()));
	connect(ui->collideEnableCheckBox, SIGNAL(toggled(bool)), mRDS, SLOT(setCollisionEnabled(bool)));
    connect(ui->senderIteratePushButton, SIGNAL(clicked()), this, SLOT(senderTimeout()));
    mDemoRayTimer->setInterval(250);
	/*PortSettings ps;
	ps.BaudRate = BAUD9600;
	ps.DataBits = DATA_8;
	ps.Parity = PAR_NONE;
	ps.StopBits = STOP_1;
	ps.FlowControl = FLOW_OFF;
	ps.Timeout_Millisec = 500;*/
	mSerial.setPortName("/dev/ttyUSB1");
	mSerial.setBaudRate(BAUD9600);
	mSerial.setDataBits(DATA_8);
	mSerial.setParity(PAR_NONE);
	mSerial.setStopBits(STOP_1);
	mSerial.setFlowControl(FLOW_OFF);
	mSerial.setTimeout(100);
	bool opened = mSerial.open(QIODevice::ReadWrite);
	if (!opened) {
		QMessageBox::critical(this, "error", QString("error opening serial port ") + mSerial.errorString());
	}
	mSerial.readAll();
	QTimer *t = new QTimer(this);
	t->setInterval(300);
	connect(t, SIGNAL(timeout()), SLOT(readData()));
	connect(this, SIGNAL(lineRead(QByteArray)), SLOT(parseData(QByteArray)));
	t->start();
	mSendTimer = new QTimer(this);
	connect(mSendTimer, SIGNAL(timeout()), SLOT(sendNextRequest()));
	mSendTimer->setSingleShot(true);
	mSendTimer->start(500);
}

RayDisplayWindow::~RayDisplayWindow()
{
	delete ui;
	delete mRDS;
}

void RayDisplayWindow::readData()
{
	mData.append(mSerial.readAll());
	if (mData.size() > 20 && mData.contains(char(0xA))) {
		int idx = mData.indexOf(char(0xA));
		QByteArray toSend = mData.left(idx);
		if (!toSend.isEmpty()) {
			emit lineRead(toSend);
		}
		mData = mData.mid(idx + 1);
	}
	sendNextRequest();
}

void RayDisplayWindow::sendNextRequest()
{
	char c = mCurrentSender;
	mSerial.write(&c, 1);
	mCurrentSender++;
	if (mCurrentSender >= 4) {
		mCurrentSender = 0;
	}
	//mSendTimer->start();
}

void RayDisplayWindow::parseData(QByteArray arr)
{
	//qDebug() << "here";
	qDebug() << arr;
	qDebug() << QString(QByteArray::fromBase64(arr).toHex());
	arr = QByteArray::fromBase64(arr);
	mRDS->lightenSender(arr.at(arr.size() - 1), arr.left(arr.size() - 1));
}

void RayDisplayWindow::on_spinBox_valueChanged(int arg1)
{
	if (ui->demoCheckBox->isChecked()) {
		mRDS->clearRays();
		mRDS->initRays(arg1);
	}
}

void RayDisplayWindow::on_demoCheckBox_toggled(bool checked)
{
	if (checked) {
		mRDS->clearRays();
		on_radioButton_toggled(ui->radioButton->isChecked());
	} else {
		mRDS->initRays(ui->rayAngleSpinBox->value());
		mDemoRayTimer->stop();
	}
}

void RayDisplayWindow::senderTimeout()
{
	mRDS->clearRays();
	mRDS->lightenSender(mCurrentSender, ui->rayAngleSpinBox->value());
	if (++mCurrentSender >= mRDS->sendersCount()) {
		mCurrentSender = 0;
	}
}

void RayDisplayWindow::on_radioButton_2_toggled(bool checked)
{
    ui->senderIteratePushButton->setEnabled(checked);
}

void RayDisplayWindow::on_radioButton_toggled(bool checked)
{
    if (checked) {
        mDemoRayTimer->start();
    } else {
        mDemoRayTimer->stop();
    }
}
