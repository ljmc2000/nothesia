#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QOpenGLWindow>
#include <QPainter>
#include <QPalette>
#include <QTimer>
#include <rtmidi/RtMidi.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum MidiMessageType{
    NOTEON=0b1001,
    NOTEOFF=0b1000,
};

struct Duration
{
    qint64 begin;
    qint64 end;
};

class NoteMap: public QHash<quint8,QRectF>
{
public:
    QColor nuetral_color;
    QColor pressed_color;
    QColor trail_color;
};

class MainWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    MainWindow(int minOctave,int maxOctave);
    ~MainWindow();

protected:
    void paintGL() override;
    void resizeGL(int,int) override;

private slots:
    void cleanup_notes();

private: //methods
    static void midiCallback(double deltatime, std::vector< unsigned char > *message, void *userData);
    int count_white_notes();
    void prepare_notemaps();
    void init_colors();

private:
    Ui::MainWindow *ui;
    RtMidiIn midiin;
    NoteMap white_note_map, black_note_map;
    qint64 playing_notes[128];
    QHash<quint8,QVector<Duration>> played_notes;
    QPainter painter;
    QTimer frameRedrawTimer,noteCleanupTimer;
	QColor background_color;

    int minOctave,maxOctave;
};
#endif // MAINWINDOW_H
