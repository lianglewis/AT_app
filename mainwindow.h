#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QtCharts>
#include <QtNetwork>
#include <QAudioBuffer>
#include <QProcess>
#include <QDebug>
#include <QHostAddress>//这个可能不需要

#define secOfRecognition 48000*2 //48000是采样率，2是一个byte是8位，而一个16位数是16位是两个byte，最后一个2是两秒的意思


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


struct WAVFILEHEADER
{
    //RIFF头
    char RiffName[4];
    unsigned long nRiffLength;

    // 数据类型标识符
    char WavName[4];

    // 格式块中的块头
    char FmtName[4];
    unsigned long nFmtLength;

    // 格式块中的块数据
    unsigned short nAudioFormat;
    unsigned short nChannleNumber;
    unsigned long nSampleRate;
    unsigned long nBytesPerSecond;
    unsigned short nBytesPerSample;
    unsigned short nBitsPerSample;

    // 数据块中的块头
    char    DATANAME[4];
    unsigned long   nDataLength;

};






class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    qint64 addWavHeader(QString catheFileName , QString wavFileName);
    void SaveFixedDurationAudio(QByteArray arrayName , QFile* outputFileName);//保存固定音频函数
    void Recoginition (qint16 *buffer);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool recognition_flag = 0;//用于判断是否按下识别按钮
    bool secondrecordflag = 0;//用于判断固定时长按钮是否按下
    bool start_flag = 0;//用于判断按开始录制按钮时，是否创建新的文件
    qint16 recognition_buffer[secOfRecognition+10000];//用于缓存2秒的识别数组
    int recognition_count = 0;
    qint64 secondDuration = 0;//用于计数存多少个字节，此变量会被一个slot函数访问
    qint64 savebytecount = 0;//用于计数保存的字节
    qint64 saveFixedFileCount = 0;

    QLabel *LabListen;//状态栏标签
    QLabel  *LabSocketState;//状态栏标签

    QTcpServer *tcpServer; //TCP服务器
    QTcpSocket *tcpSocket;//TCP通讯的Socket

    QString getLocalIP();//获取本机IP地址

    QAudioFormat format;

    QVector<double> tempBuffer1;
    QVector<double> tempBuffer2;
    QVector<double> tempBuffer3;
    QVector<double> tempBuffer4;

    QLineSeries *lineSeries1;//波形序列1
    QLineSeries *lineSeries2;//波形序列2
    QLineSeries *lineSeries3;//波形序列3
    QLineSeries *lineSeries4;//波形序列4
    const qint64  displayPointsCount=4000;

    QFile *audiofile1;
    QFile *audiofile2;
    QFile *audiofile3;
    QFile *audiofile4;

    QFile *audiowavfile1;
    QFile *audiowavfile2;
    QFile *audiowavfile3;
    QFile *audiowavfile4;
    QString selectedDir;
    QString fixedAudioDir;

    QFile *fixedaudiofile;//用于保存固定长度音频



private:
    Ui::MainWindow *ui;

private slots:
//自定义的slot
void    onNewConnection();//QTcpServer的newConnection()信号
void    onSocketStateChange(QAbstractSocket::SocketState socketState);
void    onClientConnected(); //Client Socket connected
void    onClientDisconnected();//Client Socket disconnected
void    onSocketReadyRead();//读取socket传入的数据
//UI生成的slot
    void on_actStart_triggered();
    void on_actClear_triggered();
    void on_actStop_triggered();

    void on_btnGetFile_clicked();
    void on_actionsuspend_triggered();
    void on_btnMonitor_clicked();
    void on_actQuit_triggered();
    void on_btnRecognition_clicked();
    void on_btnSaveSecond_clicked();
};


#endif // MAINWINDOW_H
