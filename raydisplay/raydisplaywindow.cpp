#include "raydisplaywindow.h"
#include "ui_raydisplaywindow.h"
#include "RayDisplayScene.h"
#include "CommConfig.h"

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
	connect(this, SIGNAL(reportRead(QByteArray)), SLOT(parseData(QByteArray)));
	connect(this, SIGNAL(calibrationRead(QByteArray)), SLOT(parseCalibration(QByteArray)));
	t->start();
	requestCalibration();
	mSendTimer = new QTimer(this);
	connect(mSendTimer, SIGNAL(timeout()), SLOT(sendNextRequest()));
	mSendTimer->setSingleShot(true);
	//mSendTimer->start(500);
	mModuleConfig.resize(20);
}

RayDisplayWindow::~RayDisplayWindow()
{
	delete ui;
	delete mRDS;
}

void RayDisplayWindow::readData()
{
	QByteArray freshData = mSerial.readAll();
	mData.append(freshData);
	// there's no point in parsing all data if there was no terminator in new data.
	if (freshData.contains(TERMINATOR)) {
		int idx;
		while ((idx = mData.indexOf(TERMINATOR)) != -1) {
			qDebug() << "there is terminator at pos" << idx << "in data" << mData;
			qDebug() << mData.left(idx + 1).toHex();
			QByteArray toSend = mData.left(idx);
			switch ((unsigned char)mData.at(0)) {
				case CALIBRATION_START:
					qDebug() << "emitting CALIBRATION_START";
					emit calibrationRead(toSend);
					break;
				case REPORT_START:
					qDebug() << "emitting REPORT_START";
					emit reportRead(toSend);
					break;
				default:
					qWarning() << "improper packet type:" << mData.at(0);
			}
			mData = mData.mid(idx + 1);
		}
		qDebug() << "no more terminator, leftover data:" << mData.toHex();
	}
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
	sendNextRequest();
}

void RayDisplayWindow::requestCalibration()
{
	char c = 'c';
	mSerial.write(&c, 1);
}

void RayDisplayWindow::parseCalibration(QByteArray arr)
{
	// skip packet id
	QByteArray data(arr.right(arr.size() - 1));
	int packetEnd;
	while ((packetEnd = data.indexOf(CALIBRATION_REPORT_END)) != -1) {
		QByteArray packet(data.left(packetEnd));
		int packetId = packet.at(0);
		// start looking for cal_pause after packet id
		int bufferEnd = packet.indexOf(CALIBRATION_PAUSE, 2);
		int start = 2;
		int end = bufferEnd - start;
		QByteArray seen(QByteArray::fromBase64(packet.mid(start, end)));

		// copy rest of data (it was limited to report size already)
		start = bufferEnd + 1;
		QByteArray config(QByteArray::fromBase64(packet.mid(start)));

		if (config.size() != seen.size()) {
			qDebug() << "config and seen have different sizes:" << config.size() << seen.size();
		}
		QByteArray modulesSeen(20, 0xFF);
		for (int i = 0, n = config.size(); i < n; i++) {
			mModuleConfig[packetId].append(qMakePair((int)config.at(i), (quint8)seen.at(i)));
			modulesSeen[config.at(i)] = seen.at(i);
		}
		mRDS->lightenSender(packetId, modulesSeen, false);
		data = data.mid(packetEnd + 1);
	}
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
