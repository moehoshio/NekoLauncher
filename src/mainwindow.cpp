#include "mainwindow.h"
#include "network.h"

// neko ui
namespace ui {

    void MainWindow::setImgbgBase() {
        static bool exec = [&]() { 
        backgroundLabel = new QLabel(this); 
        backgroundLabel->setMaximumSize(scrSize.width(),scrSize.height());
        backgroundLabel->setMinimumSize(scrSize.width()*0.35 , scrSize.height()*0.35 );
        backgroundLabel->setAutoFillBackground(true);
        backgroundLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        backgroundLabel->setScaledContents(true);
        this->setCentralWidget(backgroundLabel);
            return true; }();
    }
    void MainWindow::setVidbgBase() {
        static bool exec = [&]() { 
            player = new QMediaPlayer(this);
            videoWidget = new QVideoWidget(this);
            setCentralWidget(videoWidget);
            player->setVideoOutput(videoWidget);
            videoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
            player->setLoops(-1);
            // gridLayout->addWidget(videoWidget);
            return true; }();
    }
    void MainWindow::setBase() {
        this->setWindowTitle("Neko Launcher");
        icon = new QIcon();
        icon->addFile(QString::fromUtf8("img/ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        this->setWindowIcon(*icon);
        this->setIconSize(QSize(128, 128));
        this->resize(scrSize.width() * 0.65, scrSize.height() * 0.65);
        gridLayout = new QGridLayout(this);
        // int aleft = w * 0.15;
        // int atop = h * 0.2;
        // int awidth = w;
        // int aheight = h;
        // gridLayout->setGeometry(QRect(aleft, atop, awidth, aheight));
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

        if (exec::isUrl(backgroundPath)) {
            auto res = exec::getThreadObj().enqueue(
                [&]() {
                    neko::network net;
                    decltype(net)::Args args{backgroundPath};
                    return net.getCase(neko::networkBase::Opt::getContentType, args);
                });

            if (std::vector<std::string> vidTypes = {"mp4", "flv", "mov", "mkv"};
                exec::matchExtNames(backgroundPath, vidTypes)) {
                uVid();
                return;
            } else if (std::vector<std::string> imgTypes = {"png", "jpg", "jpeg", "webp", "gif"};
                       exec::matchExtNames(backgroundPath, imgTypes)) {
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
        std::vector<std::string>
            vidTypes = {"mp4", "flv", "mov", "mkv"},
            imgTypes = {"png", "jpg", "jpeg", "webp", "gif"};

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
        setImgbgBase();

        backgroundLabel->setPixmap(QPixmap(QString::fromUtf8(file)));
        int w = scrSize.width() * 0.65;
        int h = scrSize.height() * 0.65;
        backgroundLabel->resize(w,h);
        this->resize(w, h);
    }
    void MainWindow::setImgbg(std::shared_ptr<QByteArray> &&data) {
        setImgbgBase();
        backgroundLabel->setPixmap(QPixmap::fromImage(QImage::fromData(*data)));
        int w = scrSize.width() * 0.65;
        int h = scrSize.height() * 0.65;
        backgroundLabel->resize(w,h);
        this->resize(w, h);
    }
    void MainWindow::setVidbg(const std::string &file) {
        setVidbgBase();
        player->setSource(QUrl::fromLocalFile(QString::fromUtf8(file.c_str())));
        player->play();
    }
    void MainWindow::setVidbg(QUrl url) {
        setVidbgBase();
        player->setSource(url);
        player->play();
    }

    void MainWindow::defaultConstructor() { this->setImgbg("img/bg.png"); };

} // namespace ui

#include "mainwindow.moc"