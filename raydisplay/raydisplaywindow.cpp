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
	mCurrentSender(0),
	mCalibrationCount(3),
	mCalibrationIndex(0)
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
	mSerial.readAll();
	mInitialCalibrations.resize(mCalibrationCount);
	for (int i = 0; i < mCalibrationCount; i++) {
		mInitialCalibrations[i].moduleConfig.resize(20);
	}
	requestCalibration();
	mSendTimer = new QTimer(this);
	connect(mSendTimer, SIGNAL(timeout()), SLOT(sendNextRequest()));
	mSendTimer->setSingleShot(true);
	mSendTimer->setInterval(500);
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
			QByteArray toSend = mData.left(idx).mid(1);
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
					qWarning() << QString("improper packet type: %1").arg(QString::number(mData.at(0), 16));
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
	if (mCurrentSender >= 20) {
		mCurrentSender = 0;
	}
	//mSendTimer->start();
}

void RayDisplayWindow::parseData(const QByteArray input)
{
	//qDebug() << "here";
	//qDebug() << arr;
	//qDebug() << QString(QByteArray::fromBase64(arr).toHex());
	QByteArray arr = QByteArray::fromBase64(input);
	const int senderId = arr.at(0);
	QVector<QBitArray> seen(20, QBitArray(8, true));
	QVector<QBitArray> cal(20, QBitArray(8, true));
	const QVector<QPair<int, QBitArray> > module = mCalibration.moduleConfig.at(senderId);
	const QByteArray tempData = arr.mid(1);
	QVector<QBitArray> seenData(arr.size() - 1, QBitArray(8));
	for (int i = 0, n = seenData.size(); i < n; i++) {
		for (int j = 0; j < 8; j++) {
			seenData[i].setBit(j, tempData.at(i) & (1 << j));
		}
	}
	//Q_ASSERT_X(seenData.size() == module.size(), __func__, "seenData has different size than module!");
	if (seenData.size() != module.size()) {
		qDebug() << "size fail";
		mSendTimer->start();
		return;
	}
	for (int i = 0, n = module.size(); i < n; i++) {
		const QBitArray oldData = module.at(i).second;
		const QBitArray newData = seenData.at(i);
		const QBitArray outData = ~(oldData & ~newData);
		//const QBitArray outData = ~(newData | oldData);
		seen[module.at(i).first] = outData;
		cal[module.at(i).first] = oldData;
	}
	mRDS->lightenSender(senderId, seen, cal);
	//sendNextRequest();
	mSendTimer->start();
}

void RayDisplayWindow::requestCalibration()
{
	char c = 'c';
	mSerial.write(&c, 1);
}

void RayDisplayWindow::parseCalibration(QByteArray arr)
{
	// skip packet id
	//QByteArray data(arr.right(arr.size() - 1));
	QByteArray data(arr);
	int packetEnd;
	int sum = 0;
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
		Q_ASSERT_X(config.size() == seen.size(), __func__, "config and seen have different sizes");
		QVector<QBitArray> modulesSeen(20, QBitArray(8, true));
		auto &moduleConfig = mInitialCalibrations[mCalibrationIndex].moduleConfig;
		for (int i = 0, n = config.size(); i < n; i++) {
			QBitArray bitArr(8);
			for (int j = 0; j < 8; j++) {
				if (seen.at(i) & (1 << j)) {
					bitArr.setBit(j);
				}
			}
			moduleConfig[packetId].append(qMakePair((int)config.at(i), bitArr));
			modulesSeen[config.at(i)] = bitArr;
		}
		qDebug() << "sender" << packetId << "is seen by" << config.size() << "modules:";
		qDebug() << config.toHex();
		qDebug() << seen.toHex();
		sum += config.size();
		//mRDS->lightenSender(packetId, modulesSeen, false);
		data = data.mid(packetEnd + 1);
	}
	qDebug() << "in total there are" << sum << "configs, avg =" << (float)(sum)/20;
	if (mCalibrationCount - 1 == mCalibrationIndex) {
		// this array holds array of which modules see which how many times
		QVector<QVector<QVector<int> > > calibrationData;
		calibrationData.resize(20);
		for (int i = 0, n = calibrationData.size(); i < n; i++) {
			auto &moduleData = calibrationData[i];
			moduleData.resize(20);
			for (int j = 0, m = moduleData.size(); j < m; j++) {
				moduleData[j].resize(8);
			}
		}
		// these loops agreggate all data

		// all data
		for (int i = 0, n = mInitialCalibrations.size(); i < n; i++) {
			// sender to receivers
			const QVector<QVector<QPair<int, QBitArray> > > &initialCalibrationData = mInitialCalibrations.at(i).moduleConfig;
			for (int j = 0, m = initialCalibrationData.size(); j < m; j++) {
				// receivers
				const QVector<QPair<int, QBitArray> > &moduleData = initialCalibrationData.at(j);
				for (int k = 0, o = moduleData.size(); k < o; k++) {
					for (int x = 0; x < 8; x++) {
						if (moduleData.at(k).second.testBit(x)) {
							calibrationData[j][moduleData.at(k).first][x]++;
						}
					}
				}
			}
		}

		// the following block selects modules to be included in readouts
		{
			// senders to receivers
			for (int j = 0, m = calibrationData.size(); j < m; j++) {
				// receivers
				const QVector<QVector<int> > &moduleData = calibrationData.at(j);
				QVector<QVector<QPair<int, QBitArray> > > &moduleConfig = mCalibration.moduleConfig;
				const QVector<QPair<int, QBitArray> > &lastCalibrationData = mInitialCalibrations.at(mCalibrationCount - 1).moduleConfig.at(j);
				QVector<QPair<int, QBitArray> > t;
				for (int k = 0, o = lastCalibrationData.size(); k < o; k++) {
					QBitArray bits(8, true);
					for (int x = 0; x < 8; x++) {
						if (moduleData.at(lastCalibrationData.at(k).first).at(x) >= 2) {
							bits.clearBit(x);
						}
					}
					t.append(qMakePair(lastCalibrationData.at(k).first, bits));
				}
				mCalibration.moduleConfig.append(t);
			}
		}

		mCalibrationIndex++;
		mSendTimer->start();
	} else {
		mCalibrationIndex++;
		requestCalibration();
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
