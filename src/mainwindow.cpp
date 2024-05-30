#include "mainwindow.h"
#include "network.h"
#include <QtGui/QResizeEvent>
#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsBlurEffect>

// neko ui
namespace ui {
    constexpr qreal border = 15;
    constexpr int w = 1;

    void MainWindow::resizeEvent(QResizeEvent *event) {

        QWidget::resizeEvent(event);

        QSize oldSize = event->oldSize();
        QSize newSize = event->size();
        switch (bgType) {
            case BgType::vid:
                videoWidget->resize(newSize);
                player->pause();
                player->play();
                break;
            case BgType::img:
                backgroundLabel->resize(newSize);
                break;
            default:
                break;
        }
        // backgroundLabel->hide();
        // ->raise(); //帶到前景
        // pushButton->raise();
    }

    void MainWindow::setImgbgBase() {
        bgType = BgType::img;
        backgroundLabel = new QLabel(widget);
        backgroundLabel->setLayout(gridLayout);
        backgroundLabel->setMaximumSize(scrSize.width(), scrSize.height());
        backgroundLabel->setMinimumSize(scrSize.width() * 0.35, scrSize.height() * 0.35);
        backgroundLabel->setAutoFillBackground(true);
        backgroundLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        backgroundLabel->setScaledContents(true);
    }
    void MainWindow::setVidbgBase() {
        bgType = BgType::vid;
        player = new QMediaPlayer(widget);
        videoWidget = new QVideoWidget(widget);
        player->setVideoOutput(videoWidget);
        videoWidget->setMaximumSize(scrSize.width(), scrSize.height());
        videoWidget->setMinimumSize(scrSize.width() * 0.35, scrSize.height() * 0.35);
        videoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        player->setLoops(-1);
        // gridLayout->addWidget(videoWidget);
    }

    void MainWindow::setBase() {
        nlog::Info(FI, LI, "%s : Enter", FN);

        icon = new QIcon();
        widget = new QWidget(this);
        gridLayout = new QGridLayout(widget);
        pushButton = new QPushButton(widget);

        if (!exec::getConfigObj().GetBoolValue("more", "useSystemWindowFrame", true)) {

            barWidget = new QWidget(this);
            headbar = new HeadBar(barWidget);

            // if the headbar request win_move (which will be emitted by mouse click event)
            connect(headbar, &HeadBar::request_move_window,
                    [=, this] {
                        this->windowHandle()->startSystemMove();
                    });
            // Enable hove move event -> to change cursor when near window boarder.
            setAttribute(Qt::WA_Hover);

            bool keepR = exec::getConfigObj().GetBoolValue("more", "barKeepRight", true);
            if (keepR) {
                QWidget *spacer = new QWidget(barWidget);
                spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                headbar->addWidget(spacer);
                QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(barWidget);
                blurEffect->setBlurRadius(30);
                barWidget->setGraphicsEffect(blurEffect);
                spacer->setGraphicsEffect(blurEffect);
            }

            if (keepR) {
                auto minimize_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-minimize"), "Minimize"));
                connect(minimize_ac, &QAction::triggered,
                        [=, this]() { setWindowState(Qt::WindowMinimized); });
            } else {
                auto close_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-close"), "Close"));
                connect(close_ac, &QAction::triggered,
                        [=, this]() { QApplication::quit(); });
            }

            auto fullscreen_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-maximize"), ("fullscreen")));
            connect(fullscreen_ac, &QAction::triggered,
                    [=, this] {
             if(windowState() == Qt::WindowMaximized){
               setWindowState(Qt::WindowNoState);
             } else {
               setWindowState(Qt::WindowMaximized);
             } });

            if (keepR) {
                auto close_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-close"), "Close"));
                connect(close_ac, &QAction::triggered,
                        [=, this]() { QApplication::quit(); });
            } else {
                auto minimize_ac = headbar->addToolBtn(new QAction(QIcon::fromTheme("window-minimize"), "Minimize"));
                connect(minimize_ac, &QAction::triggered,
                        [=, this]() { setWindowState(Qt::WindowMinimized); });
            }

            this->addToolBar(headbar);
            this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

        } // not useSystemWindowFrame

        icon->addFile(QString::fromUtf8("img/ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        this->setWindowIcon(*icon);
        this->setIconSize(QSize(128, 128));
        this->setWindowTitle("Neko Launcher");

        this->setCentralWidget(widget);
        this->setAcceptDrops(true);
        this->setContentsMargins(w, w, w, w);

        widget->setLayout(gridLayout);
        widget->setMinimumSize(scrSize.width() * 0.35, scrSize.height() * 0.35);
        widget->setMaximumSize(scrSize);

        // Custom window init size
        auto expectSize = exec::getConfigObj().GetValue("main", "windowSize", "");
        auto expectSizeS = exec::matchSizesV(expectSize);

        if (std::string(expectSize) != "") {
            if (expectSizeS.max_size() == 2) {
                try {
                    int w, h;
                    for (size_t i = 0; i < 2; ++i) {
                        ((i == 0) ? w : h) = std::stoull(expectSizeS[i]);
                    }

                    QSize size{w, h};
                    this->resize(size);
                } catch (const std::exception &) {
                    this->resize(scrSize.width() * 0.65, scrSize.height() * 0.65);
                }

            } else {
                this->resize(scrSize.width() * 0.65, scrSize.height() * 0.65);
            }
        }

        pushButton->setText("hello");
        pushButton->setLayout(gridLayout);
        pushButton->setGeometry(QRect(330, 370, 75, 24));
        pushButton->setCheckable(true);
        pushButton->raise();
        pushButton->setStyleSheet(
            "QPushButton { background-color: transparent; }"
            "QPushButton:hover { background-color: #C0C0C0; }"
            "QPushButton:checked { background-color: #bfbfbf; }");

        this->setAttribute(Qt::WA_TranslucentBackground, true);

        widget->setStyleSheet("background-color: rgba(30, 30, 30, 128)");

        this->windowHandle()->startSystemMove();
    }
    void MainWindow::onVidDgCallback(std::string filePath) {
        this->setVidbg(std::move(filePath));
    }

    void MainWindow::onImgDgCallback(std::shared_ptr<QByteArray> data) {
        this->setImgbg(std::move(data));
    }
    void MainWindow::configInitBg(const CSimpleIniA &ini) {
        const char *backgroundPath = ini.GetValue("main", "background", "img/bg.jpg");
        const char *backgroundType = ini.GetValue("main", "backgroundType", "");
        nlog::Info(FI, LI, "%s : Enter , background : %s  , type : %s  ptr : %p", FN, backgroundPath, backgroundType, this);

        // Event handling can also be used here.
        connect(this, &MainWindow::resourceVideo, this, &MainWindow::onVidDgCallback);
        connect(this, &MainWindow::resourceImg, this, &MainWindow::onImgDgCallback);

        auto uVid = [&]() {
            nlog::Info(FI, LI, "%s , uVid : Enter", FN);
            exec::getThreadObj().enqueue(
                [&]() {
                    neko::network net;

                    auto name = neko::info::getTemp() + "/" + exec::generateRandomString(12);
                    decltype(net)::Args args{backgroundPath, name.c_str()};
                    bool res = net.autoRetry(neko::networkBase::Opt::downloadFile, {args});
                    if (res)
                        emit resourceVideo(name);
                });
            this->setVidbg(QUrl(backgroundPath));
        };

        auto uImg = [&]() {
            nlog::Info(FI, LI, "%s , uImg : Enter", FN);
            exec::getThreadObj().enqueue(
                [&]() {
                    using namespace neko;
                    network<QByteArray, decltype(networkBase::Write_CallbackQBA)> net;
                    decltype(net)::Args args{backgroundPath, nullptr, nullptr, false, nullptr, nullptr, nullptr, networkBase::Write_CallbackQBA};

                    auto onEmit = [this](auto &&val) {
                        emit resourceImg(val);
                    };

                    net.autoRetryGet(networkBase::Opt::getContent, {args}) | exec::move | exec::make_shared | exec::move | onEmit;
                });
            this->setImgbg(QByteArray(0));
        };

        if (std::string(backgroundType) == "video") {
            if (exec::isUrl(backgroundPath))
                uVid();
            else
                this->setVidbg(backgroundPath);
            return;
        }

        if (std::string(backgroundType) == "image") {
            if (!exec::isUrl(backgroundPath))
                setImgbg(backgroundPath);
            else
                uImg();
            return;
        }
        std::vector<std::string>
            vidTypes = {"mp4", "flv", "mov", "mkv"},
            imgTypes = {"png", "jpg", "jpeg", "webp", "gif"};

        if (exec::isUrl(backgroundPath)) {
            auto res = exec::getThreadObj().enqueue(
                [&]() {
                    neko::network net;
                    decltype(net)::Args args{backgroundPath};
                    return net.getCase(neko::networkBase::Opt::getContentType, args);
                });

            if (exec::matchExtNames(backgroundPath, vidTypes)) {
                uVid();
                return;
            } else if (exec::matchExtNames(backgroundPath, imgTypes)) {
                uImg();
                return;
            }

            std::string content_type(res.get());
            nlog::Info(FI, LI, "%s : content type is :", FN, content_type.c_str());
            if (content_type == "video") {
                uVid();
            } else if (content_type == "image") {
                uImg();
            }
            return;
        }

        // not a url

        if (exec::matchExtNames(backgroundPath, vidTypes)) {
            this->setVidbg(backgroundPath);
        } else if (exec::matchExtNames(backgroundPath, imgTypes)) {
            this->setImgbg(backgroundPath);
        } else {
            this->setImgbg("img/bg.png");
        }

        nlog::Info(FI, LI, "%s : End", FN);
    }

    void MainWindow::setImgbg(const char *file) {
        nlog::Info(FI, LI, "%s : Enter , set file version", FN);
        setImgbgBase();
        QPixmap pixmap(QString::fromUtf8(file));
        backgroundLabel->setPixmap(pixmap);
        QSize size((scrSize.width() * 0.65), (scrSize.height() * 0.65));
        backgroundLabel->resize(size);
        this->resize(size);
        pushButton->raise();
        nlog::Info(FI, LI, "%s : Exit", FN);
    }
    void MainWindow::setImgbg(std::shared_ptr<QByteArray> &&data) {
        nlog::Info(FI, LI, "%s : Enter , shared ptr data version", FN);
        setImgbgBase();
        QPixmap pixmap(QPixmap::fromImage(QImage::fromData(*data)));
        backgroundLabel->setPixmap(pixmap.scaled(scrSize.width() * 0.65, scrSize.height() * 0.65, Qt::KeepAspectRatio));
        QSize size((scrSize.width() * 0.65), (scrSize.height() * 0.65));
        backgroundLabel->resize(size);
        this->resize(size);
        pushButton->raise();

        nlog::Info(FI, LI, "%s : Exit", FN);
    }
    void MainWindow::setVidbg(const std::string &file) {
        nlog::Info(FI, LI, "%s : Enter , file : %s ", FN, file.c_str());
        setVidbgBase();
        player->setSource(QUrl::fromLocalFile(QString::fromUtf8(file.c_str())));
        QSize size((scrSize.width() * 0.65), (scrSize.height() * 0.65));
        this->resize(size);
        videoWidget->resize(size);
        player->play();
        widget->raise();
        pushButton->raise();
        nlog::Info(FI, LI, "%s : Exit", FN);
    }
    void MainWindow::setVidbg(QUrl url) {
        nlog::Info(FI, LI, "%s : Enter , Qurl : %s ", FN, url.toString().toStdString().c_str());
        setVidbgBase();
        player->setSource(url);
        QSize size((scrSize.width() * 0.65), (scrSize.height() * 0.65));
        this->resize(size);
        videoWidget->resize(size);
        player->play();
        widget->raise();
        pushButton->raise();
        nlog::Info(FI, LI, "%s : Exit", FN);
    }

    void MainWindow::defaultConstructor() {
        nlog::Info(FI, LI, "%s :Enter , ptr : %p", FN, this);
        this->setImgbg("img/bg.png");
    };

    bool MainWindow::event(QEvent *event) {
        if (!exec::getConfigObj().GetBoolValue("more", "useSystemWindowFrame", true)) {
            // When hove to edge, change cursor
            if (event->type() == QEvent::HoverMove) {
                auto p = dynamic_cast<QHoverEvent *>(event)->pos();

                if (p.y() > height() - border) {
                    if (p.x() < border) {
                        this->setCursor(Qt::SizeBDiagCursor);
                    } else if (p.x() > width() - border) {
                        this->setCursor(Qt::SizeFDiagCursor);
                    } else {
                        this->setCursor(Qt::SizeVerCursor);
                    }
                } else if ((p.y() > headbar->height()) and
                           (((p.x() > width() - border) or (p.x() < border)))) {
                    this->setCursor(Qt::SizeHorCursor);
                } else {
                    setCursor(Qt::ArrowCursor);
                }
                // if user press
            } else if (event->type() == QEvent::MouseButtonPress) {
                auto p = dynamic_cast<QMouseEvent *>(event)->pos();
                Qt::Edges edges;
                if (p.x() > width() - border)
                    edges |= Qt::RightEdge;
                if (p.x() < border)
                    edges |= Qt::LeftEdge;
                if (p.y() > height() - border)
                    edges |= Qt::BottomEdge;
                if (edges != 0) {
                    // Note: on Mac, this will return false which means isn't supported.
                    this->windowHandle()->startSystemResize(edges);
                }
            }
        }

        return QMainWindow::event(event);
    }

    void MainWindow::dragEnterEvent(QDragEnterEvent *p_event) {

        p_event->acceptProposedAction();
    }

    void MainWindow::dragMoveEvent(QDragMoveEvent *p_event) {

        p_event->acceptProposedAction();
    }

    void MainWindow::dropEvent(QDropEvent *p_event) {
        p_event->acceptProposedAction();
    }

} // namespace ui

#include "mainwindow.moc"