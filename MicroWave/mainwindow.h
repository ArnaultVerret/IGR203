#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStateMachine>
#include <QTimer>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void set_states();

signals:
    void valueChanged(int value);

public slots:
    void setValue(int value);
    void setTime();
    void initHour();
    void initMinute();
    void initPower();
    void initDuration();
    void initDefrost();
    void initMode();
    void startCooking();
    void startDefaultCooking();
    void cook();
    void increaseDuration();
    void clockCycle();

private:
    Ui::MainWindow *ui;

    int hour = 0;
    int minute = 0;
    QTimer *timer;

    int power = 100;

    int cooking_duration = 0;
    QTimer *cook_timer;

    QStateMachine * m;
    QState *idle;
    QState *set_hours;
    QState *set_minutes;
    QState *set_power;
    QState *set_duration;
    QState *set_mode;
    QState *set_defrost;
    QState *cooking;

    enum modes {MicroWave, Grill, MicroWaveGrill};
    modes mode = modes::MicroWave;
};

#endif // MAINWINDOW_H
