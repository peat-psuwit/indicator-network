#include "menustack.h"

#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeProperty>

MenuStack::MenuStack(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_animation(0)
{
    QObject::connect(this, SIGNAL(widthChanged()), this, SLOT(onSizeChanged()));
    QObject::connect(this, SIGNAL(heightChanged()), this, SLOT(onSizeChanged()));
}

MenuStack::~MenuStack()
{
    delete m_animation;
    m_menus.clear();
}

int MenuStack::count() const
{
    return m_menus.count();
}

void MenuStack::onSizeChanged()
{
    foreach(QDeclarativeItem *menu, m_menus) {
        menu->setWidth(width());
        menu->setHeight(height());
    }
}

void MenuStack::onAnimationValueChanged(const QVariant &value)
{
    if (m_oldItem)
        m_oldItem->setX(value.toReal() - width());
}

void MenuStack::onAnimationFowardFinished()
{
    if(m_oldItem)
        m_oldItem->setVisible(false);
}

void MenuStack::onAnimationBackFinished()
{
    m_animation->targetObject()->deleteLater();
    m_oldItem = m_menus.last();
}

void MenuStack::pushMenu(QDeclarativeComponent *component)
{
    if (!component)
        return;

    QDeclarativeContext *ctx = QDeclarativeEngine::contextForObject(this);
    QObject *menuObject = component->create(ctx);
    Q_ASSERT(menuObject);

    if (m_menus.size())
        m_oldItem = m_menus.last();
    else
        m_oldItem = 0;

    QDeclarativeItem *menu = qobject_cast<QDeclarativeItem*>(menuObject);
    menu->setParentItem(this);
    menu->setWidth(width());
    menu->setHeight(height());
    m_menus.append(menu);


    // Finalize current Animation
    if (m_animation) {
        m_animation->setLoopCount(m_animation->loopCount());
        delete m_animation;
    }

    // Start a new animation
    m_animation = new QPropertyAnimation(menu, "x");
    m_animation->setDuration(300);
    m_animation->setStartValue(width());
    m_animation->setEndValue(0);
    connect(m_animation, SIGNAL(valueChanged(QVariant)), SLOT(onAnimationValueChanged(QVariant)));
    connect(m_animation, SIGNAL(finished()), SLOT(onAnimationFowardFinished()));

    emit countChanged();
    m_animation->start();
}

void MenuStack::popMenu()
{
    if ((m_menus.size() <= 0) || !m_oldItem)
        return;

    // Finalize current Animation
    if (m_animation) {
        m_animation->setLoopCount(m_animation->loopCount());
        delete m_animation;
    }

    QDeclarativeItem *last = m_menus.pop();
    m_oldItem = m_menus.last();
    m_oldItem->setVisible(true);

    m_animation = new QPropertyAnimation(last, "x");
    m_animation->setDuration(300);
    m_animation->setStartValue(0);
    m_animation->setEndValue(width());
    connect(m_animation, SIGNAL(valueChanged(QVariant)), SLOT(onAnimationValueChanged(QVariant)));
    connect(m_animation, SIGNAL(finished()), SLOT(onAnimationBackFinished()));
    m_oldItem->setVisible(true);

    emit countChanged();
    m_animation->start();
}