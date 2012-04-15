#include "raydisplaywindow.h"
#include "ui_raydisplaywindow.h"
#include "RayDisplayScene.h"

#include <QTimer>

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
}

RayDisplayWindow::~RayDisplayWindow()
{
	delete ui;
	delete mRDS;
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
        mDemoRayTimer->start();
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
