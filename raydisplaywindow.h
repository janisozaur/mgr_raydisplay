#ifndef RAYDISPLAYWINDOW_H
#define RAYDISPLAYWINDOW_H

#include <QMainWindow>

namespace Ui {
class RayDisplayWindow;
}

class RayDisplayWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit RayDisplayWindow(QWidget *parent = 0);
    ~RayDisplayWindow();
    
private:
    Ui::RayDisplayWindow *ui;
};

#endif // RAYDISPLAYWINDOW_H
