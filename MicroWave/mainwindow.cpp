#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "transitions.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // state machine
    set_states();

    // clock
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::clockCycle);
    timer->start(1000*60);

    // Clock for cooking
    cook_timer = new QTimer(this);
    connect(cook_timer, &QTimer::timeout, this, &MainWindow::cook);

    // Connect start button when cooking
    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(increaseDuration()));

    // dial connection
    connect(ui->dial, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    setTime();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_states(){
    m = new QStateMachine();

    //Idle
    idle = new QState();
    m->addState(idle);

    //Clock
    set_hours = new QState();
    set_minutes = new QState();
    m->addState(set_hours);
    m->addState(set_minutes);

    //Power
    set_power = new QState();
    m->addState(set_power);

    //Duration
    set_duration = new QState();
    m->addState(set_duration);

    //Mode
    set_mode = new QState();
    m->addState(set_mode);

    //Defrost
    set_defrost = new QState();
    m->addState(set_defrost);

    //Cooking
    cooking = new QState();
    m->addState(cooking);

    // from idle state
    addTrans(idle, set_hours, ui->clockButton, SIGNAL(clicked()), this, SLOT(initHour()));
    addTrans(idle, set_power, ui->powerButton, SIGNAL(clicked()), this, SLOT(initPower()));
    addTrans(idle, set_mode, ui->modeButton, SIGNAL(clicked()), this, SLOT(initMode()));
    addTrans(idle, set_defrost, ui->defrostButton, SIGNAL(clicked()), this, SLOT(initDefrost()));
    addTrans(idle, cooking, ui->startButton, SIGNAL(clicked()), this, SLOT(startDefaultCooking()));

    // from set_hours
    addTrans(set_hours, idle, ui->stopbutton, SIGNAL(clicked()));
    addTrans(set_hours, set_minutes, ui->clockButton, SIGNAL(clicked()), this, SLOT(initMinute()));

    // from set_minutes
    addTrans(set_minutes, idle, ui->stopbutton, SIGNAL(clicked()));
    addTrans(set_minutes, idle, ui->clockButton, SIGNAL(clicked()), this, SLOT(setTime()));

    // from set_power
    addTrans(set_power, idle, ui->stopbutton, SIGNAL(clicked()));
    addTrans(set_power, set_duration, ui->powerButton, SIGNAL(clicked()), this, SLOT(initDuration()));

    // from set_mode
    addTrans(set_mode, idle, ui->stopbutton, SIGNAL(clicked()));
    addTrans(set_mode, set_duration, ui->modeButton, SIGNAL(clicked()));

    // from set_defrost
    addTrans(set_defrost, idle, ui->stopbutton, SIGNAL(clicked()));
    addTrans(set_defrost, cooking, ui->startButton, SIGNAL(clicked()), this, SLOT(startCooking()));

    // from set_duration
    addTrans(set_duration, idle, ui->stopbutton, SIGNAL(clicked()));
    addTrans(set_duration, cooking, ui->startButton, SIGNAL(clicked()), this, SLOT(startCooking()));

    // from cooking
    addTrans(cooking, idle, ui->stopbutton, SIGNAL(clicked()), this, SLOT(setTime()));

    m->setInitialState(idle);
    m->start();
}

void MainWindow::setValue(int value){
    auto state = m->configuration();
    if (state.contains(idle)){
        setTime();
    }
    else if (state.contains(set_hours)) {
        int time = (value*24)/100;
        ui->textBrowser->setText(QString::number(time)+" hours");
        hour = time;
    }
    else if (state.contains(set_minutes)) {
        int time = (value*60)/100;
        ui->textBrowser->setText(QString::number(time)+" minutes");
        minute = time;
    }
    else if (state.contains(set_power)) {
        ui->textBrowser->setText(QString::number(value+1)+"% power");
        power = value+1;

    }
    else if (state.contains(set_duration)) {
        int minutes = value/4;
        int seconds = value%4 * 15;
        ui->textBrowser->setText(QString::number(minutes)+" minutes and " +
                                 QString::number(seconds)+" seconds");
        cooking_duration = 60*minutes + seconds;
    }
    else if (state.contains(set_defrost)) {
        int grams = 50*value; // value in grams
        ui->textBrowser->setText(QString::number(grams)+" grams");
        cooking_duration = grams/20; // choose randomly...
    }
    else if (state.contains(set_mode)){
        int v = value/49;
        if (v==0){
            mode = modes::MicroWave;
            ui->textBrowser->setText("mode : Microwave");
        }
        else if (v==1){
            mode = modes::Grill;
            ui->textBrowser->setText("mode : Grill");
        }
        else{
            mode = modes::MicroWaveGrill;
            ui->textBrowser->setText("mode : Microwave+Grill");
        }
    }
}

void MainWindow::initHour(){
    ui->textBrowser->setText(QString::number(hour) + " hours");
}

void MainWindow::initMinute(){
    ui->textBrowser->setText(QString::number(minute) + " minutes");
}

void MainWindow::initPower(){
    ui->textBrowser->setText(QString::number(power) + "% power");
}

void MainWindow::initDuration(){
    int minutes = cooking_duration/60;
    int seconds = cooking_duration%60;
    ui->textBrowser->setText(QString::number(minutes)+" minutes and " +
                             QString::number(seconds)+" seconds");
}

void MainWindow::initDefrost(){
    ui->textBrowser->setText(QString::number(100)+" grams");
}

void MainWindow::initMode(){
    ui->textBrowser->setText("mode : Microwave");
    mode = modes::MicroWave;
}

void MainWindow::startCooking(){
    int minutes = cooking_duration/60;
    int seconds = cooking_duration%60;
    ui->textBrowser->setText(QString::number(minutes)+" minutes and " +
                             QString::number(seconds)+" seconds");
    cook_timer->start(1000);
}

void MainWindow::startDefaultCooking(){
    power = 100;
    cooking_duration = 60;
    startCooking();
}

void MainWindow::cook(){
    if (! m->configuration().contains(cooking))
        return;

    cooking_duration--;
    if (cooking_duration < 0){
        cooking_duration=0;
        ui->textBrowser->setText("Finished ! Press Stop");
        return;
    }

    int minutes = cooking_duration/60;
    int seconds = cooking_duration%60;
    ui->textBrowser->setText(QString::number(minutes)+" minutes and " +
                             QString::number(seconds)+" seconds");
    cook_timer->start(1000); // timer of 1 second that'll change the cooking duration every timeout.
}

void MainWindow::increaseDuration(){
    if (m->configuration().contains(cooking))
        cooking_duration+=60;
    int minutes = cooking_duration/60;
    int seconds = cooking_duration%60;
    ui->textBrowser->setText(QString::number(minutes)+" minutes and " +
                             QString::number(seconds)+" seconds");
}

void MainWindow::setTime(){
    ui->textBrowser->setText(QString::number(hour)+" hours, "+QString::number(minute) + " minutes");
}

void MainWindow::clockCycle(){
    if (m->configuration().contains(set_hours) || m->configuration().contains(set_minutes))
        return;
    minute++;
    if (minute == 60){
        hour++;
        minute=0;
    }
    if (hour == 24)
        hour = 0;
    timer->start(1000*60);
    if (m->configuration().contains(idle))
        setTime();
}
