#include "FBController.h"
#include "management.h"
#include "log.h"

void FBController::setFramebufferID(int fbId){
    if(_framebufferID != -1){
        QDEBUG << "Cannot change a framebuffer ID once set!! (Trying to change" << _framebufferID << "into" << fbId << ") (rejected.)";
        return;
    }
    _framebufferID = fbId;
    qtfb::management::registerController(fbId, QPointer(this));
}

int FBController::framebufferID() const {
    return _framebufferID;
}

bool FBController::active() const {
    return _active;
}

void FBController::setActive(bool active){
    _active = active;
    emit activeChanged();
    markedUpdate();
}

FBController::~FBController(){
    qtfb::management::unregisterController(_framebufferID);
}

void FBController::paint(QPainter *painter) {
    isMidPaint = true;
    QDEBUG << "FB Repaint triggered for " << _framebufferID << ". Status: " << _active;
    // Do we have an SHM associated?
    if(this->image && this->_active) {
        // Cool. Paint it.
        if(_allowScaling) {
            painter->drawImage(QRect(0, 0, width(), height()), *image, image->rect());
        } else {
            painter->drawImage(0, 0, *image);
        }
    } else {
        QDEBUG << "Placeholder";
        QFont font = painter->font();
        font.setPointSize(50);
        font.setBold(true);
        painter->setFont(font);
        QRect rect(0, 0, width(), height());
        painter->fillRect(rect, QColor(255, 255, 0));
        painter->drawText(rect, "Unbound Framebuffer " + QString::number(_framebufferID), Qt::AlignCenter | Qt::AlignTop);
    }
    isMidPaint = false;
}

void FBController::associateSHM(QImage *image) {
    this->image = image;
    int key = _framebufferID;
    QMetaObject::invokeMethod(this, [this, key]() {
        if(!qtfb::management::isControllerAssociated(key)) {
            // The framebuffer connection was terminated as we were
            // waiting for the event loop to process this request.
            this->image = nullptr;
            this->setActive(false);
            return;
        }
        this->setActive(this->image != nullptr);
    }, Qt::QueuedConnection);
}

void FBController::markedUpdate(const QRect &rect) {
    isMidPaint = true;
    if(_allowScaling && image) {
        update(QRect(
           (rect.x() / image->width()) * this->width(),
           (rect.y() / image->height()) * this->height(),
           (rect.width() / image->width()) * this->width(),
           (rect.height() / image->height()) * this->height()
        ));
    } else {
        update(rect);
    }
}

void FBController::setAllowScaling(bool a){
    _allowScaling = a;
}

bool FBController::allowScaling() const {
    return _allowScaling;
}
