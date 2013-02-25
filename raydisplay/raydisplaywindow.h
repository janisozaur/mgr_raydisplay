#ifndef RAYDISPLAYWINDOW_H
#define RAYDISPLAYWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QBitArray>
#include <qextserialport.h>

namespace Ui {
	class RayDisplayWindow;
}

class RayDisplayScene;
class QTimer;

struct CalibrationData
{
	QVector<QVector<QPair<int, QBitArray> > > moduleConfig;
};

class RayDisplayWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit RayDisplayWindow(QWidget *parent = 0);
    ~RayDisplayWindow();
    
private slots:
	void on_spinBox_valueChanged(int arg1);
	void on_demoCheckBox_toggled(bool checked);
	void senderTimeout();

    void on_radioButton_2_toggled(bool checked);

    void on_radioButton_toggled(bool checked);

	void readData();
	void parseData(const QByteArray input);
	void sendNextRequest();
	void requestCalibration();
	void parseCalibration(QByteArray arr);

signals:
	void reportRead(QByteArray arr);
	void calibrationRead(QByteArray arr);

private:
    Ui::RayDisplayWindow *ui;
    RayDisplayScene *mRDS;
	QTimer *mDemoRayTimer;
	int mCurrentSender;
	QextSerialPort mSerial;
	QByteArray mData;
	QTimer *mSendTimer;
	QVector<CalibrationData> mInitialCalibrations;
	CalibrationData mCalibration;
	const int mCalibrationCount;
	int mCalibrationIndex;
};

#endif // RAYDISPLAYWINDOW_H
