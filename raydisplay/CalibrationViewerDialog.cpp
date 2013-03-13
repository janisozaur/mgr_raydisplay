#include "CalibrationViewerDialog.h"
#include "ui_CalibrationViewerDialog.h"

CalibrationViewerDialog::CalibrationViewerDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CalibrationViewerDialog)
{
	ui->setupUi(this);
	ui->calibrationGraphicsView->setScene(new QGraphicsScene(this));
}

CalibrationViewerDialog::~CalibrationViewerDialog()
{
	delete ui;
}

void CalibrationViewerDialog::setSizes(const QVector<QVector<QPointF> > &receivers, const QVector<QVector<QPointF> > &senders)
{
	const int n = receivers.size();
	for (int i = 0; i < n; i++) {
		const QVector<QPointF> &receiverSide = receivers.at(i);
		for (int j = 0; j < receiverSide.size(); j++) {
			QGraphicsEllipseItem *r = ui->calibrationGraphicsView->scene()->addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::black), 2));
			r->setPos(receiverSide.at(j));
			mReceivers.append(r);
		}
	}
	const int m = senders.size();
	for (int i = 0; i < m; i++) {
		const QVector<QPointF> &sendersSide = senders.at(i);
		for (int j = 0; j < sendersSide.size(); j++) {
			QGraphicsEllipseItem *r = ui->calibrationGraphicsView->scene()->addEllipse(0, 0, 5, 5, QPen(QBrush(Qt::red), 2));
			r->setPos(sendersSide.at(j));
			mSenders.append(r);
		}
	}
}

void CalibrationViewerDialog::setCalibrations(const QVector<CalibrationData> calibrations)
{
	mCalibrations = calibrations;
	ui->calibrationSelectList->clear();
	for (int i = 0; i < mCalibrations.size(); i++) {
		ui->calibrationSelectList->addItem(QString::number(i));
	}
}

void CalibrationViewerDialog::on_calibrationSelectList_currentRowChanged(int currentRow)
{
	ui->moduleSelectList->clear();
	if (currentRow != -1) {
		for (int i = 0; i < mCalibrations.at(currentRow).moduleConfig.size(); i++) {
			ui->moduleSelectList->addItem(QString::number(i));
		}
	}
}

void CalibrationViewerDialog::on_moduleSelectList_currentRowChanged(int currentRow)
{
	qDeleteAll(mRays);
	mRays.clear();
	if (currentRow != -1) {
		const QVector<QVector<QPair<int, QBitArray> > > moduleConfig = mCalibrations.at(ui->calibrationSelectList->currentRow()).moduleConfig;
		const QVector<QPair<int, QBitArray> > thisModule = moduleConfig.at(currentRow);
		for (int i = 0; i < thisModule.size(); i++) {
			const QPair<int, QBitArray> &receiver = thisModule.at(i);
			const int receiverModuleId = receiver.first * 8;
			for (int j = 0; j < 8; j++) {
				if (!receiver.second.testBit(j)) {
					const QPointF p1 = mSenders.at(currentRow)->scenePos();
					const int idx = receiverModuleId + j;
					const QPointF p2 = mReceivers.at(idx)->scenePos();
					const QLineF line(p1, p2);
					QGraphicsLineItem *l = ui->calibrationGraphicsView->scene()->addLine(line, QPen(Qt::black));
					mRays << l;
				}
			}
		}
	}
}
