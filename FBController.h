#pragma once

#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QString>
#include <QImage>
#include <QJsonDocument>
#include <QPainter>
#include <QJsonObject>
#include <QJsonValue>
#include <QQuickPaintedItem>

class FBController : public QQuickPaintedItem
{
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(int framebufferID READ framebufferID WRITE setFramebufferID)
    Q_PROPERTY(bool allowScaling READ allowScaling WRITE setAllowScaling)
    Q_OBJECT
public:
    explicit FBController(QQuickItem *parent = nullptr) : QQuickPaintedItem(parent) { /*setOpaquePainting(true);*/ }
    virtual ~FBController();

    void setFramebufferID(int fbID);
    int framebufferID() const;

    void setAllowScaling(bool a);
    bool allowScaling() const;

    bool active() const;

    void markedUpdate(const QRect &rect = QRect());
    void setActive(bool active); // NOT QML ACCESSIBLE!
    bool isMidPaint;
    virtual void paint(QPainter *painter);
    void associateSHM(QImage *image);
signals:
    void activeChanged();

private:
    int _framebufferID = -1;
    bool _active = false;
    bool _allowScaling = false;

    QImage *image = nullptr;
};
