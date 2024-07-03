#pragma once

#include "network.h"
#include "headbar.h"

#include <memory>
#include <string>
#include <chrono>

#include <QtCore/QList>
#include <QtCore/QMetaObject>
#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>

#include <QtGui/QPainterPath>

#include <QtGui/QScreen>

#include <QtGui/QGuiApplication>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>

#include "SimpleIni/SimpleIni.h"

namespace ui {

    class MainWindow : public QMainWindow {
        Q_OBJECT
    private:
        QWidget * widget;
        QLabel *backgroundLabel;
        QIcon *icon;
        QGridLayout *gridLayout;
        QMediaPlayer *player;
        QVideoWidget *videoWidget;
        QPushButton *pushButton;
        HeadBar * headbar;
        QWidget * barWidget;
        const QSize scrSize = QGuiApplication::primaryScreen()->size();

        enum class BgType{
            vid,
            img
        };
        BgType bgType;
    public:
        MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
            nlog::Info(FI, LI, "%s : default Constructor", FN);
            setBase();
            defaultConstructor();
            setMouseTracking(true);
        }

        MainWindow(const CSimpleIniA &ini, QWidget *parent = nullptr) : QMainWindow(parent) {

            nlog::Info(FI, LI, "%s : config Constructor  , obj empty : %s", FN, exec::boolTo<const char *>(ini.IsEmpty()));
            setBase();
            setMouseTracking(true);

            if (!ini.IsEmpty())
                configInitBg(ini);
            else
                defaultConstructor();
        }

        void setBase();
        void defaultConstructor();
        void configInitBg(const CSimpleIniA &ini);

        void setImgbgBase();
        void setVidbgBase();

        void setImgbg(const char *file);
        void setImgbg(std::shared_ptr<QByteArray> &&data);
        void setVidbg(const std::string &file);
        void setVidbg(QUrl url);


        void resizeEvent(QResizeEvent *event) override;

        bool event(QEvent *event) override;
        void dropEvent(QDropEvent *p_event) override;
        void dragEnterEvent(QDragEnterEvent *p_event) override;
        void dragMoveEvent(QDragMoveEvent *p_event) override;

        ~MainWindow() noexcept = default;


        void onVidDgCallback(std::string filePath);

        void onImgDgCallback(std::shared_ptr<QByteArray> data);

    signals:
        void resourceVideo(std::string filePath);
        void resourceImg(std::shared_ptr<QByteArray> data);
    };
} // namespace ui
#include "mainwindow.moc"