#include "mainwindow.h"
#include <QCoreApplication>
#include <QFileInfoList>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    videoLayout();                      //视频播放器布局初始化

    mediaPlayerInit();                  //媒体初始化

    scanVideoFiles();                   //扫描本地视频

    pushButton[0]->setCheckable(true);  //设置播放、全屏按钮初始状态
    pushButton[4]->setCheckable(true);

    connect(pushButton[0], &QPushButton::clicked,
            this,
            &MainWindow::btn_play_clicked);         //播放按钮槽函数
    connect(pushButton[1], &QPushButton::clicked,
            this,
            &MainWindow::btn_next_clicked);         //切换按钮槽函数
    connect(pushButton[2], &QPushButton::clicked,
            this,
            &MainWindow::btn_volmedown_clicked);    //音量减按钮槽函数
    connect(pushButton[3], &QPushButton::clicked,
            this,
            &MainWindow::btn_volmeup_clicked);      //音量加按钮槽函数
    connect(pushButton[4], &QPushButton::clicked,
            this,
            &MainWindow::btn_fullscreen_clicked);   //全屏按钮槽函数

    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            this,
            SLOT(listWidgetCliked(QListWidgetItem*)));          //播放列表项目被选中

    connect(videoPlayer, SIGNAL(stateChanged(QMediaPlayer::State)),
            this,
            SLOT(mediaPlayerStateChanged(QMediaPlayer::State)));//播放器状态改变

    connect(mediaPlaylist, SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(mediaPlaylistCurrentIndexChanged(int)));       //媒体改变，更新播放列表的选中行

    connect(videoPlayer, SIGNAL(durationChanged(qint64)),
            this,
            SLOT(musicPlayerDurationChanged(qint64)));          //(总时间）

    connect(videoPlayer, SIGNAL(positionChanged(qint64)),
            this,
            SLOT(mediaPlayerPositionChanged(qint64)));          //(当前时间）

    connect(durationSlider, &QSlider::sliderReleased,
            this,
            &MainWindow::durationSliderReleased);               //滑块移动时更新视频显示位置

    connect(volumeSlider, &QSlider::sliderReleased,
            this,
            &MainWindow::volumeSliderReleased);                 //滑块移动时更新音量
}

MainWindow::~MainWindow()
{
}

void MainWindow::videoLayout()
{
    this->setGeometry(0, 0, 800, 480);
    //this->setMinimumSize(800, 480);
    //this->setMaximumSize(800, 480);
    QPalette pal;                               //设置调色板格式
    pal.setColor(QPalette::WindowText, Qt::white);

    for (int i = 0; i < 3; i++) {               //水平容器初始化
        hWidget[i] = new QWidget();
        hWidget[i]->setAutoFillBackground(true);
        hBoxLayout[i] = new QHBoxLayout();
    }

    for (int i = 0; i < 2; i++) {               //垂直容器初始化
        vWidget[i] = new QWidget();
        vWidget[i]->setAutoFillBackground(true);
        vBoxLayout[i] = new QVBoxLayout();
    }

    for (int i = 0; i < 2; i++) {               //Label
        label[i] = new QLabel();
    }

    for (int i = 0; i < 5; i++) {               //按钮初始化
        pushButton[i] = new QPushButton();
        pushButton[i]->setMaximumSize(44, 44);
        pushButton[i]->setMinimumSize(44, 44);
    }

    vWidget[0]->setObjectName("vWidget0");      //设置对象名称
    vWidget[1]->setObjectName("vWidget1");
    hWidget[1]->setObjectName("hWidget1");
    hWidget[2]->setObjectName("hWidget2");
    pushButton[0]->setObjectName("btn_play");
    pushButton[1]->setObjectName("btn_next");
    pushButton[2]->setObjectName("btn_volumedown");
    pushButton[3]->setObjectName("btn_volumeup");
    pushButton[4]->setObjectName("btn_screen");

    QFont font;
    font.setPixelSize(18);

    label[0]->setFont(font);                        //Label初始化（播放时间）
    label[1]->setFont(font);

    //pal.setColor(QPalette::WindowText, Qt::white);
    label[0]->setPalette(pal);
    label[1]->setPalette(pal);

    label[0]->setText("00:00");
    label[1]->setText("/00:00");

    durationSlider = new QSlider(Qt::Horizontal);   //Slider初始化（播放时间条）
    durationSlider->setMaximumHeight(15);
    durationSlider->setObjectName("durationSlider");

    volumeSlider = new QSlider(Qt::Horizontal);     //Slider初始化（音量条）
    volumeSlider->setRange(0, 100);
    volumeSlider->setMaximumWidth(80);
    volumeSlider->setObjectName("volumeSlider");
    volumeSlider->setValue(50);

    listWidget = new QListWidget();                 //ListWidget初始化（播放列表）
    listWidget->setObjectName("listWidget");
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);        //内容太大无法装下时，显示滚动条，否则不显示
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //listWidget->setFocusPolicy(Qt::NoFocus);
    videoWidget = new QVideoWidget();               //视频显示窗口
    videoWidget->setStyleSheet("border-image: none;"
                               "background: transparent;"
                               "border:none");

    /**************************视频区域布局*****************************/

    vWidget[0]->setMinimumSize(300, 480);
    vWidget[0]->setMaximumWidth(300);
    videoWidget->setMinimumSize(500, 480);

    hBoxLayout[0]->addWidget(videoWidget);
    hBoxLayout[0]->addWidget(vWidget[0]);

    hWidget[0]->setLayout(hBoxLayout[0]);
    hBoxLayout[0]->setContentsMargins(0, 0, 0, 0);

    setCentralWidget(hWidget[0]);

    /**************************播放列表区域布局*****************************/

    QSpacerItem *vSpacer0 = new QSpacerItem(0, 80,
                                            QSizePolicy::Minimum,
                                            QSizePolicy::Maximum);
    vBoxLayout[0]->addWidget(listWidget);
    vBoxLayout[0]->addSpacerItem(vSpacer0);
    vBoxLayout[0]->setContentsMargins(0, 0, 0, 0);//页边空白

    vWidget[0]->setLayout(vBoxLayout[0]);

    hWidget[1]->setMaximumHeight(15);
    hWidget[2]->setMinimumHeight(65);
    vBoxLayout[1]->addWidget(hWidget[1]);
    vBoxLayout[1]->addWidget(hWidget[2]);
    vBoxLayout[1]->setAlignment(Qt::AlignCenter);//对齐方式

    vWidget[1]->setLayout(vBoxLayout[1]);
    vWidget[1]->setParent(this);
    vWidget[1]->setGeometry(0, this->height() - 80, this->width(), 80);
    vBoxLayout[1]->setContentsMargins(0, 0, 0, 0);

    vWidget[1]->raise();    //位于最上层

    /**************************滑块区域布局*****************************/

    hBoxLayout[1]->addWidget(durationSlider);
    hBoxLayout[1]->setContentsMargins(0, 0, 0, 0);
    hWidget[1]->setLayout(hBoxLayout[1]);

    /* H2布局 */    QSpacerItem *hSpacer0 = new QSpacerItem(300, 80,
                                            QSizePolicy::Expanding,
                                            QSizePolicy::Maximum);
    hBoxLayout[2]->addSpacing(20);
    hBoxLayout[2]->addWidget(pushButton[0]);
    hBoxLayout[2]->addSpacing(10);
    hBoxLayout[2]->addWidget(pushButton[1]);
    hBoxLayout[2]->addSpacing(10);
    hBoxLayout[2]->addWidget(pushButton[2]);
    hBoxLayout[2]->addWidget(volumeSlider);
    hBoxLayout[2]->addWidget(pushButton[3]);
    hBoxLayout[2]->addWidget(label[0]);
    hBoxLayout[2]->addWidget(label[1]);
    hBoxLayout[2]->addSpacerItem(hSpacer0);
    hBoxLayout[2]->addWidget(pushButton[4]);
    hBoxLayout[2]->addSpacing(20);
    hBoxLayout[2]->setContentsMargins(0, 0, 0, 0);
    hBoxLayout[2]->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    hWidget[2]->setLayout(hBoxLayout[2]);
}

void MainWindow::mediaPlayerInit()              //媒体初始化
{
    videoPlayer = new QMediaPlayer(this);
    mediaPlaylist = new QMediaPlaylist(this);
    mediaPlaylist->clear();
    videoPlayer->setPlaylist(mediaPlaylist);                //设置视频播放器的列表为mediaPlaylist
    videoPlayer->setVideoOutput(videoWidget);               //设置视频输出窗口
    mediaPlaylist->setPlaybackMode(QMediaPlaylist::Loop);   //设置播放模式，Loop是列循环
    videoPlayer->setVolume(50);                             //设置默认软件音量为50%
}

void MainWindow::scanVideoFiles()               //扫描视频文件
{
    QDir dir(QCoreApplication::applicationDirPath() + "/myVideo");
    QDir dirbsolutePath(dir.absolutePath());
    /* 如果目录存在 */
    if (dirbsolutePath.exists()) {
        /* 定义过滤器 */
        QStringList filter;
        /* 包含所有xx后缀的文件 */
        filter << "*.mp4" << "*.mkv" << "*.wmv" << "*.avi" << "*.ts";
        /* 获取该目录下的所有文件 */
        QFileInfoList files = dirbsolutePath.entryInfoList(filter, QDir::Files);
        /* 遍历 */
        for (int i = 0; i < files.count(); i++) {
            MediaObjectInfo info;
            /* 使用utf-8编码 */
            info.fileName = QString::fromUtf8(files.at(i).fileName().toUtf8().data());
            info.filePath = QString::fromUtf8(files.at(i).filePath().toUtf8().data());
            /* 媒体列表添加视频 */
            if (mediaPlaylist->addMedia( QUrl::fromLocalFile(info.filePath))) {
                /* 添加到容器数组里储存 */
                mediaObjectInfo.append(info);
                /* 添加视频名字至列表 */
                listWidget->addItem(info.fileName);
            } else {
                qDebug()<< mediaPlaylist->errorString().toUtf8().data() << endl;
                qDebug()<< "  Error number:" << mediaPlaylist->error() << endl;
            }
        }
    }
}

void MainWindow::btn_play_clicked()     /* 播放按钮 */
{
    int state = videoPlayer->state();   //获取视频播放状态
    switch (state) {
    case QMediaPlayer::StoppedState:    //视频状态如果为：停止
        videoPlayer->play();            //播放
        break;

    case QMediaPlayer::PlayingState:    //视频状态如果为：正在播放
        videoPlayer->pause();           //停止播放
        break;

    case QMediaPlayer::PausedState:     //视频状态如果为：暂停
        videoPlayer->play();            //播放
        break;
    }
}

void MainWindow::btn_next_clicked()         /* 切换按钮 */
{
    videoPlayer->stop();                    //停止播放
    int count = mediaPlaylist->mediaCount();//获取播放列表的视频数
    if (0 == count)                         //如果视频数为0
        return;                             //中断函数执行，返回调用函数处

    mediaPlaylist->next();
    videoPlayer->play();
}

void MainWindow::btn_volmedown_clicked()    /* 音量减按钮 */
{
    volumeSlider->setValue(volumeSlider->value() - 5);      //点击每次音量-5
    videoPlayer->setVolume(volumeSlider->value());
}

void MainWindow::btn_volmeup_clicked()      /* 音量加按钮 */
{
    volumeSlider->setValue(volumeSlider->value() + 5);      //点击每次音量+5
    videoPlayer->setVolume(volumeSlider->value());
}

void MainWindow::btn_fullscreen_clicked()   /* 全屏按钮 */
{
    vWidget[0]->setVisible(!pushButton[4]->isChecked());    //全屏/非全屏操作
}

void MainWindow::listWidgetCliked(QListWidgetItem *item)    //播放列表点击
{
    videoPlayer->stop();
    mediaPlaylist->setCurrentIndex(listWidget->row(item));
    videoPlayer->play();
}

void MainWindow::mediaPlayerStateChanged(QMediaPlayer::State state) //播放器状态改变
{
    switch (state) {
    case QMediaPlayer::StoppedState:
        pushButton[0]->setChecked(false);
        break;

    case QMediaPlayer::PlayingState:
        pushButton[0]->setChecked(true);
        break;

    case QMediaPlayer::PausedState:
        pushButton[0]->setChecked(false);
        break;
    }
}

void MainWindow::mediaPlaylistCurrentIndexChanged(int index)    //媒体改变，更新播放列表的选中行
{
    if (-1 == index)
        return;
    listWidget->setCurrentRow(index);    //设置列表正在播放的项
}

void MainWindow::musicPlayerDurationChanged(qint64 duration)        //视频时间滑块(总时间）
{
    durationSlider->setRange(0, duration / 1000);
    int second  = duration / 1000;
    int minute = second / 60;
    second %= 60;

    QString mediaDuration;
    mediaDuration.clear();

    if (minute >= 10)
        mediaDuration = QString::number(minute, 10);
    else
        mediaDuration = "0" + QString::number(minute, 10);

    if (second >= 10)
        mediaDuration += ":" + QString::number(second, 10);
    else
        mediaDuration += ":0" + QString::number(second, 10);

    label[1]->setText("/" + mediaDuration);                 //显示媒体总长度时间
}

void MainWindow::mediaPlayerPositionChanged(qint64 position)        //视频时间滑块(当前时间）
{
    if (!durationSlider->isSliderDown())
        durationSlider->setValue(position / 1000);

    int second  = position / 1000;
    int minute = second / 60;
    second %= 60;

    QString mediaPosition;
    mediaPosition.clear();

    if (minute >= 10)
        mediaPosition = QString::number(minute, 10);
    else
        mediaPosition = "0" + QString::number(minute, 10);

    if (second >= 10)
        mediaPosition += ":" + QString::number(second, 10);
    else
        mediaPosition += ":0" + QString::number(second, 10);

    label[0]->setText(mediaPosition);                       //显示现在播放的时间
}

void MainWindow::durationSliderReleased()
{
    videoPlayer->setPosition(durationSlider->value() * 1000);   //设置媒体播放的位置
}

void MainWindow::volumeSliderReleased()
{

    videoPlayer->setVolume(volumeSlider->value());              //设置音量
}


void MainWindow::resizeEvent(QResizeEvent *event)//重写窗口大小设置事件
{
    Q_UNUSED(event);
    vWidget[1]->setGeometry(0, this->height() - 80, this->width(), 80);
}
