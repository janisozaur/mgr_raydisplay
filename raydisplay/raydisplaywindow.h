#ifndef RAYDISPLAYWINDOW_H
#define RAYDISPLAYWINDOW_H

#include <QMainWindow>
#include <QVector>

namespace Ui {
	class RayDisplayWindow;
}

class RayDisplayScene;
class QTimer;

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

private:
    Ui::RayDisplayWindow *ui;
    RayDisplayScene *mRDS;
	QTimer *mDemoRayTimer;
	int mCurrentSender;
};

#endif // RAYDISPLAYWINDOW_H
