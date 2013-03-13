#ifndef CALIBRATIONVIEWERDIALOG_H
#define CALIBRATIONVIEWERDIALOG_H

#include <QDialog>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>

#include "raydisplaywindow.h"

namespace Ui {
class CalibrationViewerDialog;
}

class CalibrationViewerDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit CalibrationViewerDialog(QWidget *parent = 0);
	~CalibrationViewerDialog();

public slots:
	void setSizes(const QVector<QVector<QPointF>> &receivers, const QVector<QVector<QPointF>> &senders);
	void setCalibrations(const QVector<CalibrationData> calibrations);
	
private slots:
	void on_calibrationSelectList_currentRowChanged(int currentRow);

	void on_moduleSelectList_currentRowChanged(int currentRow);

private:
	Ui::CalibrationViewerDialog *ui;
	QVector<QGraphicsEllipseItem *> mReceivers;
	QVector<QGraphicsEllipseItem *> mSenders;
	QVector<QGraphicsLineItem *> mRays;
	QVector<CalibrationData> mCalibrations;
	//QVector<QVector<QPointF>> mReceivers;
	//QVector<QVector<QPointF>> mSenders;
};

#endif // CALIBRATIONVIEWERDIALOG_H
