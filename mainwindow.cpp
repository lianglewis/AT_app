#include "mainwindow.h"
#include "ui_mainwindow.h"


//简单的孤立词检测,检测到对应的音频将会发送给MCU做音频的播放
void MainWindow::Recoginition (qint16 *buffer)
{
    int volumeThersholdConunt = 0;//计算2s内超过阈值的点数有多少
    for(int i = 0 ; i < secOfRecognition ; i++)
    {
        if(buffer[i]>ui->spinThreshold->value())
        {
            volumeThersholdConunt++;
        }
    }
    ui->plainTextEdit->appendPlainText(QString::number(volumeThersholdConunt));
    ui->plainTextEdit->appendPlainText("识别完成");
    if(volumeThersholdConunt > ui->spinThreeWordCount->value())
    {
        ui->plainTextEdit->appendPlainText("识别结果为：label3");
        tcpSocket->write("c");
    }
    else if(volumeThersholdConunt > ui->spinTwoWordCount->value())
    {
        ui->plainTextEdit->appendPlainText("识别结果为：label2");
        tcpSocket->write("b");}
    else if(volumeThersholdConunt > ui->spinOneWordCount->value())
    {
        ui->plainTextEdit->appendPlainText("识别结果为：label1");
        tcpSocket->write("a");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("没有识别到任何语句");
    }
};

//固定时长文件保存
void MainWindow::SaveFixedDurationAudio(QByteArray arrayName , QFile* outputFileName)
{
    qint64 size = arrayName.size();
//    ui->plainTextEdit->appendPlainText(QString::number(size));
    if(savebytecount + size <= secondDuration)
    {
        savebytecount += size ;
        outputFileName->write(arrayName);
//        ui->plainTextEdit->appendPlainText("test");
    }
    else if (savebytecount + size > secondDuration)
    {
        qint64 bytesToWrite = secondDuration - savebytecount ;
        outputFileName->write(arrayName.constData(), bytesToWrite);
        savebytecount = 0;
        secondrecordflag = 0;
        outputFileName->close();
        addWavHeader(fixedAudioDir + "/" +QString::number(saveFixedFileCount) + ".raw" ,fixedAudioDir + "/" +QString::number(saveFixedFileCount) + ".wav");//将pcm文件转为wav文件
        fixedaudiofile->remove();//删除pcm文件，只保留wav文件
        delete fixedaudiofile;//释放堆区内存
        ui->plainTextEdit->appendPlainText("录制完毕");
        ui->btnSaveSecond->setEnabled(true);
    }
}

MainWindow::MainWindow(QWidget *parent)//构造函数
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("人工喉录音工具");//设置window标题

    //创建显示图表1
    QChart *chart = new QChart;
    chart->setTitle("CH1:mems mic");
    ui->chartView->setChart(chart);
    //序列
    lineSeries1= new QLineSeries();
    chart->addSeries(lineSeries1);
    QValueAxis *axisX = new QValueAxis;  //坐标轴
    axisX->setRange(0, displayPointsCount); //chart显示4000个采样点数据
    axisX->setLabelFormat("%g");
//    axisX->setTitleText("帧数");
    QValueAxis *axisY = new QValueAxis;  //坐标轴
    axisY->setRange(-1, 1);
    axisY->setTitleText("level");
    chart->addAxis(axisX,Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);
    lineSeries1->attachAxis(axisX);
    lineSeries1->attachAxis(axisY);
    chart->legend()->hide();

    //创建显示图表2
    QChart *chart2 = new QChart;
    chart2->setTitle("CH2:FREE");
    ui->chartView_2->setChart(chart2);
    //序列
    lineSeries2= new QLineSeries();
    chart2->addSeries(lineSeries2);
    QValueAxis *axisX2 = new QValueAxis;  //坐标轴
    axisX2->setRange(0, displayPointsCount); //chart显示4000个采样点数据
    axisX2->setLabelFormat("%g");
//    axisX2->setTitleText("帧数");
    QValueAxis *axisY2 = new QValueAxis;  //坐标轴
    axisY2->setRange(-1, 1);
    axisY2->setTitleText("level");
    chart2->addAxis(axisX2,Qt::AlignBottom);
    chart2->addAxis(axisY2,Qt::AlignLeft);
    lineSeries2->attachAxis(axisX2);
    lineSeries2->attachAxis(axisY2);
    chart2->legend()->hide();

    //创建显示图表3
    QChart *chart3 = new QChart;
    chart3->setTitle("CH3 Artificial throat");
    ui->chartView_3->setChart(chart3);
    //序列
    lineSeries3= new QLineSeries();
    chart3->addSeries(lineSeries3);
    QValueAxis *axisX3 = new QValueAxis;  //坐标轴
    axisX3->setRange(0, displayPointsCount); //chart显示4000个采样点数据
    axisX3->setLabelFormat("%g");
//    axisX3->setTitleText("帧数");
    QValueAxis *axisY3 = new QValueAxis;  //坐标轴
    axisY3->setRange(-1, 1);
    axisY3->setTitleText("R level");
    chart3->addAxis(axisX3,Qt::AlignBottom);
    chart3->addAxis(axisY3,Qt::AlignLeft);
    lineSeries3->attachAxis(axisX3);
    lineSeries3->attachAxis(axisY3);
    chart3->legend()->hide();

    //创建显示图表4
    QChart *chart4 = new QChart;
    chart4->setTitle("CH4:FREE");
    ui->chartView_4->setChart(chart4);
    //序列
    lineSeries4= new QLineSeries();
    chart4->addSeries(lineSeries4);
    QValueAxis *axisX4 = new QValueAxis;  //坐标轴
    axisX4->setRange(0, displayPointsCount); //chart显示4000个采样点数据
    axisX4->setLabelFormat("%g");
//    axisX4->setTitleText("Frame");
    QValueAxis *axisY4 = new QValueAxis;  //坐标轴
    axisY4->setRange(-1, 1);
    axisY4->setTitleText("level");
    chart4->addAxis(axisX4,Qt::AlignBottom);
    chart4->addAxis(axisY4,Qt::AlignLeft);
    lineSeries4->attachAxis(axisX4);
    lineSeries4->attachAxis(axisY4);
    chart4->legend()->hide();



    LabListen=new QLabel("监听状态:");
    LabListen->setMinimumWidth(150);
    ui->statusBar->addWidget(LabListen);

    LabSocketState=new QLabel("Socket状态：");
    LabSocketState->setMinimumWidth(200);
    ui->statusBar->addWidget(LabSocketState);

    //    //更改ip地址，如果用手机的话，因为手机网关地址会发生改变，所以暂时来说没法搞固定地址
    //    //netsh interface ip set address \"以太网\" static 192.168.10.4 255.255.255.0 192.168.10.1
    //        QProcess *process = new QProcess();
    //        QString cmd = "netsh interface ip set address ";
    //        cmd += "\"";     //网卡名称要加双引号
    //        cmd += "以太网";
    //        cmd += "\" ";

    //        cmd += "static ";

    //        cmd += "192.168.10.4"; //IP
    //        cmd += " ";

    //        cmd += "255.255.255.0"; //子网掩码
    //        cmd += " ";

    //        cmd += "192.168.10.1"; //网关

    //        qDebug()<< "cmd = " << cmd;

    //        process->start(cmd);   //执行dos命令
    //        process->waitForFinished(); //等待执行完成
    //        delete process;

    QString localIP=getLocalIP();//本机IP
    this->setWindowTitle(this->windowTitle()+"----本机IP："+localIP);
    ui->comboIP->addItem(localIP);
    //设置音频缓存区格式
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setChannelCount(1);
    format.setCodec("pcm");
    format.setSampleRate(48000);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);

    //防止用户误触
    //    ui->actStart->setEnabled(false);
    ui->actStop->setEnabled(false);
    ui->actionsuspend->setEnabled(false);
    ui->btnSaveSecond->setEnabled(false);

    tcpServer=new QTcpServer(this);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewConnection()
{
    tcpSocket = tcpServer->nextPendingConnection();//创建socket

    connect(tcpSocket, SIGNAL(connected()),
            this, SLOT(onClientConnected()));

    connect(tcpSocket, SIGNAL(disconnected()),
            this, SLOT(onClientDisconnected()));

    connect(tcpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this,SLOT(onSocketStateChange(QAbstractSocket::SocketState)));

    connect(tcpSocket,SIGNAL(readyRead()),
            this,SLOT(onSocketReadyRead()));
}

void MainWindow::onClientConnected()//实际测试发现这一行并不会被打印出来
{//客户端接入时
    ui->plainTextEdit->appendPlainText("**client socket connected");
    ui->plainTextEdit->appendPlainText("**peer address:"+
                                       tcpSocket->peerAddress().toString());
    ui->plainTextEdit->appendPlainText("**peer port:"+
                                       QString::number(tcpSocket->peerPort()));
    //    ui->actStart->setEnabled(true);
    //    ui->actStop->setEnabled(false);
    //    ui->actionsuspend->setEnabled(false);
}

void MainWindow::onClientDisconnected()
{//客户端断开连接时
    ui->plainTextEdit->appendPlainText("**client socket disconnected");
    tcpSocket->deleteLater();
}

void MainWindow::onSocketStateChange(QAbstractSocket::SocketState socketState)
{//socket状态变化时
    switch(socketState)
    {
    case QAbstractSocket::UnconnectedState:
        LabSocketState->setText("scoket状态：UnconnectedState");
        break;
    case QAbstractSocket::HostLookupState:
        LabSocketState->setText("scoket状态：HostLookupState");
        break;
    case QAbstractSocket::ConnectingState:
        LabSocketState->setText("scoket状态：ConnectingState");
        break;

    case QAbstractSocket::ConnectedState:
        LabSocketState->setText("scoket状态：ConnectedState");
        break;

    case QAbstractSocket::BoundState:
        LabSocketState->setText("scoket状态：BoundState");
        break;

    case QAbstractSocket::ClosingState:
        LabSocketState->setText("scoket状态：ClosingState");
        break;

    case QAbstractSocket::ListeningState:
        LabSocketState->setText("scoket状态：ListeningState");
    }
}


void MainWindow::onSocketReadyRead()//实际波形显示的时候发现仍然会卡顿，不知道是不是esp32s3发送缓存区太小导致的
{//读取pcm流到每个音频缓存
    //    ui->plainTextEdit->appendPlainText("正在读取pcm流");
    qint64 count = tcpSocket->bytesAvailable();//计算有多少个字节可以读
    //    ui->plainTextEdit->appendPlainText(QString::number(count));
    qint64 loopNum = (count / 8) /2 ;//循环赋值多少次,不用考虑没有读出来剩下的几个数据，将它放在buffer中下次读即可，每次只读一半的数据,保证下次就算卡了也有数据可读
    //    ui->plainTextEdit->appendPlainText(QString::number(loopNum));
    //    QByteArray buffer = tcpSocket->readAll();

    QByteArray m_audioData1;
    QByteArray m_audioData2;
    QByteArray m_audioData3;
    QByteArray m_audioData4;

    while(loopNum--)
    {
        m_audioData1.append(tcpSocket->read(2));
        m_audioData2.append(tcpSocket->read(2));
        m_audioData3.append(tcpSocket->read(2));
        m_audioData4.append(tcpSocket->read(2));
    }

    // chart1
    QAudioBuffer m_audiobuffer1 = QAudioBuffer(m_audioData1,format);//先拿一个带有音频格式的buffer来接这个数据
    int nFrameCount1 = m_audiobuffer1.frameCount() / 3;//计算收到的帧数，并且降采样3倍
    qint16  * data = m_audiobuffer1.data<qint16>();//把pcm数据转化为int16来显示
    // // 识别部分，正式识别演示时用第三个通道，故此通道不进行识别
    //    if(recognition_flag == 1)
    //    {
    //        if((recognition_count + nFrameCount1) <= secOfRecognition)
    //        {
    //            for(int i = 0 ; i < nFrameCount1 ; i++)
    //            {
    //                recognition_buffer[recognition_count + i] = data[i];
    //                recognition_count ++;
    //            }
    // //            ui->plainTextEdit->appendPlainText(QString::number(recognition_count));
    //        }
    //        else if((recognition_count + nFrameCount1) > secOfRecognition)
    //        {
    //            for(int i = 0 ; i < secOfRecognition - recognition_count - 1; i++)
    //            {
    //                recognition_buffer[recognition_count + i] = data[i];
    //                recognition_count++;
    //            }
    // //            ui->plainTextEdit->appendPlainText(QString::number(recognition_count));
    //            Recoginition(recognition_buffer);
    //            recognition_flag = 0;//清空标志位
    //            recognition_count = 0;//清空计数
    //        }
    //    }
    const qreal max = qreal(SHRT_MAX);//int16的最大值，即255，此数据用于后续的归一化
    tempBuffer1.fill(0,nFrameCount1);//固定大小
    for(int i = 0 ; i < nFrameCount1 ; i++)//降采样赋值显示，把原本48khz音频降为16khz数据点进行显示，保存时仍然是48khz,相当于将原来的波形显示速度变慢三倍
    {
        tempBuffer1[i] = data[3*i] / max;
    }
    QVector<QPointF> oldPoints = lineSeries1->pointsVector();
    QVector<QPointF> points;
    if (oldPoints.count() < displayPointsCount)
    {
        if(displayPointsCount - oldPoints.count() >= nFrameCount1)
        {
            points = lineSeries1->pointsVector();
        }
        else
        {
            int nMoveCount = nFrameCount1 - (displayPointsCount - oldPoints.count());
            for (int i = nMoveCount; i < oldPoints.count(); i++)
                points.append(QPointF(i - nMoveCount, oldPoints.at(i).y()));
        }
    }
    else
    {
        for (int i = nFrameCount1; i < oldPoints.count(); i++)
            points.append(QPointF(i - nFrameCount1, oldPoints.at(i).y()));
    }
    qint64 size = points.count();
    for (int k = 0; k < nFrameCount1; k++)
        points.append(QPointF(k + size,tempBuffer1[k]));
    lineSeries1->replace(points);


    // chart2
    QAudioBuffer m_audiobuffer2 = QAudioBuffer(m_audioData2,format);
    int nFrameCount2 = m_audiobuffer2.frameCount() / 3;
    qint16  * data2 = m_audiobuffer2.data<qint16>();
    const qreal max2 = qreal(SHRT_MAX);
    tempBuffer2.fill(0,nFrameCount2);
    for(int i = 0 ; i < nFrameCount2 ; i++)
    {
        tempBuffer2[i] = data2[3*i] / max2;
    }
    QVector<QPointF> oldPoints2 = lineSeries2->pointsVector();
    QVector<QPointF> points2;
    if (oldPoints.count() < displayPointsCount)
    {
        if(displayPointsCount - oldPoints2.count() >= nFrameCount2)
        {
            points2 = lineSeries2->pointsVector();
        }
        else
        {
            int nMoveCount2 = nFrameCount2 - (displayPointsCount - oldPoints2.count());
            for (int i = nMoveCount2; i < oldPoints2.count(); i++)
                points2.append(QPointF(i - nMoveCount2, oldPoints2.at(i).y()));
        }
    }
    else
    {
        for (int i = nFrameCount2; i < oldPoints2.count(); i++)
            points2.append(QPointF(i - nFrameCount2, oldPoints2.at(i).y()));
    }
    qint64 size2 = points2.count();
    for (int k = 0; k < nFrameCount2; k++)
        points2.append(QPointF(k + size2,tempBuffer2[k]));
    lineSeries2->replace(points2);

    // chart3-人工喉通道，这个通道的显示进行降采样并且坐标变换回电阻的大小，识别分类不进行降采样，数据越多越好分类
    QAudioBuffer m_audiobuffer3 = QAudioBuffer(m_audioData3,format);
    int nFrameCount3 = m_audiobuffer3.frameCount() ;
    int nFrameCount3Display = m_audiobuffer3.frameCount() / 3;
    qint16  * data3 = m_audiobuffer3.data<qint16>();
    //反过来计算一下阻值大小，公式待定
//    qint64 R =
    if(recognition_flag == 1)//识别分类判断
    {
        if((recognition_count + nFrameCount3) <= secOfRecognition)
        {
            for(int i = 0 ; i < nFrameCount3 ; i++)
            {
                recognition_buffer[recognition_count + i] = data3[i];
                recognition_count ++;
            }
            //            ui->plainTextEdit->appendPlainText(QString::number(recognition_count));
        }
        else if((recognition_count + nFrameCount3) > secOfRecognition)
        {
            for(int i = 0 ; i < secOfRecognition - recognition_count - 1; i++)
            {
                recognition_buffer[recognition_count + i] = data3[i];
                recognition_count++;
            }
            //            ui->plainTextEdit->appendPlainText(QString::number(recognition_count));
            Recoginition(recognition_buffer);
            recognition_flag = 0;//清空标志位
            recognition_count = 0;//清空计数
        }
    }
    const qreal max3 = qreal(SHRT_MAX);
    tempBuffer3.fill(0,nFrameCount3Display);
    for(int i = 0 ; i < nFrameCount3Display ; i++)
    {
        tempBuffer3[i] = data3[i*3] / max3;
    }
    QVector<QPointF> oldPoints3 = lineSeries3->pointsVector();
    QVector<QPointF> points3;
    if (oldPoints.count() < displayPointsCount)
    {
        if(displayPointsCount - oldPoints3.count() >= nFrameCount3Display)
        {
            points3 = lineSeries3->pointsVector();
        }
        else
        {
            int nMoveCount3 = nFrameCount3Display - (displayPointsCount - oldPoints3.count());
            for (int i = nMoveCount3; i < oldPoints3.count(); i++)
                points3.append(QPointF(i - nMoveCount3, oldPoints3.at(i).y()));
        }
    }
    else
    {
        for (int i = nFrameCount3Display; i < oldPoints3.count(); i++)
            points3.append(QPointF(i - nFrameCount3Display, oldPoints3.at(i).y()));
    }
    qint64 size3 = points3.count();
    for (int k = 0; k < nFrameCount3Display; k++)
        points3.append(QPointF(k + size3,tempBuffer3[k]));
    lineSeries3->replace(points3);

    // chart4
    QAudioBuffer m_audiobuffer4 = QAudioBuffer(m_audioData4,format);
    int nFrameCount4 = m_audiobuffer4.frameCount() / 3;
    qint16  * data4 = m_audiobuffer4.data<qint16>();
    const qreal max4 = qreal(SHRT_MAX);
    tempBuffer4.fill(0,nFrameCount4);
    for(int i = 0 ; i < nFrameCount4 ; i++)
    {
        tempBuffer4[i] = data4[3*i] / max4;
    }
    QVector<QPointF> oldPoints4 = lineSeries4->pointsVector();
    QVector<QPointF> points4;
    if (oldPoints.count() < displayPointsCount)
    {
        if(displayPointsCount - oldPoints4.count() >= nFrameCount4)
        {
            points4 = lineSeries4->pointsVector();
        }
        else
        {
            int nMoveCount4 = nFrameCount4 - (displayPointsCount - oldPoints4.count());
            for (int i = nMoveCount4; i < oldPoints4.count(); i++)
                points4.append(QPointF(i - nMoveCount4, oldPoints4.at(i).y()));
        }
    }
    else
    {
        for (int i = nFrameCount4; i < oldPoints4.count(); i++)
            points4.append(QPointF(i - nFrameCount4, oldPoints4.at(i).y()));
    }
    qint64 size4 = points4.count();
    for (int k = 0; k < nFrameCount4; k++)
        points4.append(QPointF(k + size4,tempBuffer4[k]));
    lineSeries4->replace(points4);
    //    ui->plainTextEdit->appendPlainText("执行完成");
    //    将数据写道文件中
    audiofile1->write(m_audioData1);
    audiofile2->write(m_audioData2);
    audiofile3->write(m_audioData3);
    audiofile4->write(m_audioData4);
    if(secondrecordflag == 1)
    {
        SaveFixedDurationAudio(m_audioData3,fixedaudiofile);
    }

}

void MainWindow::on_actStart_triggered()
{//发送开始录音命令
    //第一次开始时创建文件，后面除非点击结束，都不重复创建在选择的目录下创建音频文件
    if(start_flag == 0)
    {
        audiofile1 = new QFile(selectedDir + ("/audio1.raw"));
        audiofile1->open(QIODevice::ReadWrite);
        audiofile2 = new QFile(selectedDir + ("/audio2.raw"));
        audiofile2->open(QIODevice::ReadWrite);
        audiofile3 = new QFile(selectedDir + ("/audio3.raw"));
        audiofile3->open(QIODevice::ReadWrite);
        audiofile4 = new QFile(selectedDir + ("/audio4.raw"));
        audiofile4->open(QIODevice::ReadWrite);
        start_flag = 1;
    }

    //发送开始命令给mcu client
    tcpSocket->write("1");
    //    tcpSocket->flush();
    ui->actStart->setEnabled(false);
    ui->actStop->setEnabled(true);
    ui->actionsuspend->setEnabled(true);
}

QString MainWindow::getLocalIP()
{//获取本机IPv4地址
    QString hostName=QHostInfo::localHostName();//本地主机名
    QHostInfo   hostInfo=QHostInfo::fromName(hostName);
    QString   localIP="";

    QList<QHostAddress> addList=hostInfo.addresses();//

    if (!addList.isEmpty())
        for (int i=0;i<addList.count();i++)
        {
            QHostAddress aHost=addList.at(i);
            if (QAbstractSocket::IPv4Protocol==aHost.protocol())
            {
                localIP=aHost.toString();
                break;
            }
        }
    return localIP;
}

void MainWindow::on_actClear_triggered()
{
    ui->plainTextEdit->clear();
}

void MainWindow::on_actStop_triggered()
{//结束一场录音
    tcpSocket->write("5");
    tcpSocket->flush();
    ui->actStop->setEnabled(false);
    ui->actStart->setEnabled(true);
    ui->actionsuspend->setEnabled(false);
    //将原来的pcm文件转为wav格式
    audiowavfile1 = new QFile(selectedDir + "/audio1.wav");
    addWavHeader(selectedDir + ("/audio1.raw"),selectedDir + "/audio1.wav");
    audiowavfile2 = new QFile(selectedDir + "/audio1.wav");
    addWavHeader(selectedDir + ("/audio2.raw"),selectedDir + "/audio2.wav");
    audiowavfile3 = new QFile(selectedDir + "/audio3.wav");
    addWavHeader(selectedDir + ("/audio3.raw"),selectedDir + "/audio3.wav");
    audiowavfile4 = new QFile(selectedDir + "/audio1.wav");
    addWavHeader(selectedDir + ("/audio4.raw"),selectedDir + "/audio4.wav");
    //关闭本次录音的raw文件并删除
    audiofile1->close();
    audiofile2->close();
    audiofile3->close();
    audiofile4->close();
    audiofile1->remove();
    audiofile2->remove();
    audiofile3->remove();
    audiofile4->remove();
    LabListen->setText("本次录音结束，已完成pcm到wav的转化");
    start_flag = 0; //恢复下一次开始创建新的文件
}


qint64 MainWindow::addWavHeader(QString catheFileName , QString wavFileName)
{
    // 开始设置WAV的文件头
    WAVFILEHEADER WavFileHeader;
    qstrcpy(WavFileHeader.RiffName,"RIFF");
    qstrcpy(WavFileHeader.WavName, "WAVE");
    qstrcpy(WavFileHeader.FmtName, "fmt ");
    qstrcpy(WavFileHeader.DATANAME, "data");
    WavFileHeader.nFmtLength = 16;//FMT长度
    WavFileHeader.nAudioFormat = 1;//PCM编码
    WavFileHeader.nChannleNumber = 1;//声道数
    WavFileHeader.nSampleRate = 48000;//采样率
    WavFileHeader.nBitsPerSample = 2;//每个sample的字节数
    WavFileHeader.nBitsPerSample = 16;//采样位数

    QFile cacheFile(catheFileName);
    QFile wavFile(wavFileName);

    if (!cacheFile.open(QIODevice::ReadWrite))
    {
        return -1;
    }
    if (!wavFile.open(QIODevice::WriteOnly))
    {
        return -2;
    }

    int nSize = sizeof(WavFileHeader);
    qint64 nFileLen = cacheFile.bytesAvailable();

    WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
    WavFileHeader.nDataLength = nFileLen;

    // 先将wav文件头信息写入，再将音频数据写入;
    wavFile.write((char *)&WavFileHeader, nSize);
    wavFile.write(cacheFile.readAll());

    cacheFile.close();
    wavFile.close();
    cacheFile.remove();

    return nFileLen;
}



void MainWindow::on_btnGetFile_clicked()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,tr("Open Directory"),"/home",QFileDialog::ShowDirsOnly);
    if (!selectedDir.isEmpty())
        ui->editOutputDir->setText(selectedDir);

}


void MainWindow::on_actionsuspend_triggered()
{
    tcpSocket->write("2");
    tcpSocket->flush();
    ui->actStart->setEnabled(true);
    ui->actStop->setEnabled(true);
    ui->actionsuspend->setEnabled(false);
}




void MainWindow::on_btnMonitor_clicked()
{//开始监听
    selectedDir = ui->editOutputDir->text().trimmed();
    if (selectedDir.isEmpty())//选择文件夹
    {
        QMessageBox::critical(this,"Error","Please select the directory of the output files.");
        return;
    }
    //防止用户误触
    ui->btnMonitor->setEnabled(false);
    ui->btnSaveSecond->setEnabled(true);
    //    ui->editOutputDir->setEnabled(false);


    QString     IP=ui->comboIP->currentText();//IP地址
    quint16     port=ui->spinPort->value();//端口
    QHostAddress    addr(IP);
    tcpServer->listen(addr,port);
    ui->plainTextEdit->appendPlainText("**开始监听...");
    ui->plainTextEdit->appendPlainText("**服务器地址："
                                       +tcpServer->serverAddress().toString());
    ui->plainTextEdit->appendPlainText("**服务器端口："
                                       +QString::number(tcpServer->serverPort()));
    LabListen->setText("监听状态：正在监听");
}


void MainWindow::on_actQuit_triggered()
{//停止监听并退出
    if (tcpServer->isListening()) //tcpServer正在监听
    {
        tcpServer->close();//停止监听
        LabListen->setText("监听状态：已停止监听");
    }
    tcpSocket->deleteLater();
    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText("已停止");
    MainWindow::close();
}


void MainWindow::on_btnRecognition_clicked()
{
    recognition_flag = 1;
    ui->plainTextEdit->appendPlainText("正在识别...");
}


void MainWindow::on_btnSaveSecond_clicked()
{//保存固定秒数的音频
    ui->plainTextEdit->appendPlainText(QString("录制第%1次音频").arg(saveFixedFileCount + 1));
    saveFixedFileCount ++ ;
    secondDuration = (ui->spinDuration->value()) * 48000 * 2 ;//一共要保存这么多个字节
    ui->btnSaveSecond->setEnabled(false);
    //在用户所选目录下创建一个文件保存固定时长音频的文件夹,如果存在则不创建
    QString folderName = QString("%1s_audio").arg(ui->spinDuration->value());
    QDir outputFolder(selectedDir);
    if(!outputFolder.exists(folderName)){
        outputFolder.mkdir(folderName);
    }
    //创建音频文件，每保存一次创建一个新的音频文件
    fixedAudioDir = selectedDir + QString("/%1").arg(folderName);
    fixedaudiofile = new QFile(fixedAudioDir + "/" +QString::number(saveFixedFileCount) + ".raw");
    fixedaudiofile->open(QIODevice::ReadWrite);
    secondrecordflag = 1;//标记为1，应该最后把标志位置为1，因为qt是每个信号和槽是一个线程同时操作的原因，在前面置为一有可能导致文件夹还没创建就被写入，程序崩溃，但是放到后面还是会崩溃
}

