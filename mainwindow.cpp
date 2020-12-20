#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#define RATE 5

MainWindow::MainWindow(int minOctave, int maxOctave)
    : QOpenGLWindow(NoPartialUpdate)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->minOctave=minOctave;
    this->maxOctave=maxOctave;
    prepare_notemaps();
    midiin.setClientName("nothesia");
    midiin.openVirtualPort();
    midiin.setCallback(&MainWindow::midiCallback,this);

    init_colors();


    frameRedrawTimer.setInterval(1);
    connect(&frameRedrawTimer, SIGNAL(timeout()),
            this, SLOT(update()));
    frameRedrawTimer.start();

    for(int i=0; i<127; i++)
    {
        played_notes[i];
    }
    noteCleanupTimer.setInterval(1000);
    connect(&noteCleanupTimer, &QTimer::timeout,
            this, &MainWindow::cleanup_notes);
    noteCleanupTimer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_colors()
{
    black_note_map.nuetral_color=Qt::black;
    black_note_map.pressed_color=QColor(50,50,50);
    black_note_map.trail_color=Qt::red;

    white_note_map.nuetral_color=Qt::white;
    white_note_map.pressed_color=Qt::gray;
    white_note_map.trail_color=Qt::green;
}

void MainWindow::resizeGL(int width, int height)
{
    QOpenGLWindow::resizeGL(width,height);
    prepare_notemaps();
}

void MainWindow::paintGL()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    painter.begin(this);
    painter.setPen(Qt::black);
    painter.fillRect(0,0,width(),height(),QPalette::Window);

    for(NoteMap note_map: {white_note_map,black_note_map})
    {
        for(QHash<quint8,QRectF>::Iterator it=note_map.begin(); it!=note_map.end(); it++)
        {
            qint64 duration=playing_notes[it.key()];
            if(duration==0)
            {
                painter.fillRect(it.value(),note_map.nuetral_color);
                painter.drawRect(it.value());
            }

            else
            {
                quint32 height=(now-duration)/RATE;
                QRectF note=QRectF(QPointF(it.value().left(),it.value().top()),QPointF(it.value().right(),it.value().top()-height));
                painter.fillRect(it.value(),note_map.pressed_color);
                painter.drawRect(it.value());
                painter.fillRect(note,note_map.trail_color);
                painter.drawRect(note);
            }

            for(QVector<Duration>::iterator jt=played_notes[it.key()].begin(); jt!=played_notes[it.key()].end(); jt++)
            {
                quint32 h1=(now-jt->begin)/RATE;
                quint32 h2=(now-jt->end)/RATE;
                QRectF note=QRectF(QPointF(it.value().left(),it.value().top()-h2),QPointF(it.value().right(),it.value().top()-h1));
                painter.fillRect(note,note_map.trail_color);
                painter.drawRect(note);
            }
        }
    }
    painter.end();
}

void MainWindow::prepare_notemaps()
{
    float w_note_width=this->width()/(7*(maxOctave-minOctave));
    float w_note_height=w_note_width*5;
    float b_note_width=w_note_width*.75;
    float b_note_height=b_note_width*4;
    float b_note_offset=b_note_width/2;
    float y=height()-(w_note_height);
    float tl=0;

    for(quint8 n=12*minOctave; n<=12*maxOctave; n++)
    {
        playing_notes[n]=0;

        switch(n%12)
        {
            case 0:
            case 2:
            case 4:
            case 5:
            case 7:
            case 9:
            case 11:
                white_note_map[n]=QRectF(tl,y,w_note_width,w_note_height);
                tl+=w_note_width;
                break;
            default:
                black_note_map[n]=QRectF(tl-b_note_offset,y,b_note_width,b_note_height);
                break;
        }
    }
}

void MainWindow::cleanup_notes()
{
    qint64 now=QDateTime::currentMSecsSinceEpoch();
    for(int i=0; i<127; i++)
    {
        if(!played_notes[i].isEmpty())
        if(played_notes[i].last().end<now-10000)
        {
            played_notes[i].clear();
        }
    }
}

void MainWindow::midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    MainWindow *self=(MainWindow*)userData;

    switch(message->at(0)>>4)
    {
        case NOTEOFF:
        noteoff:
        {
            qint64 begin = self->playing_notes[message->at(1)];
            qint64 end = QDateTime::currentMSecsSinceEpoch();
            self->played_notes[message->at(1)].append({begin,end});
            if(self->played_notes[message->at(1)].length()>32) self->played_notes[message->at(1)].removeAt(0);
            self->playing_notes[message->at(1)]=0;
            break;
        }

        case NOTEON:
        {
            if(message->at(2) == 0) goto noteoff;
            self->playing_notes[message->at(1)]=QDateTime::currentMSecsSinceEpoch();
            break;
        }

        default:
            break;
    }
}