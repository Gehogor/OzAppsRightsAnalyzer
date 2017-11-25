#define _PUSHBUTTONCOLOR_CPP_
#include "PushButtonColor.h"
#undef _PUSHBUTTONCOLOR_CPP_

// Qt
#include <QColorDialog>


// Constructor ---------------------------------------------------------------//
PushButtonColor::PushButtonColor(const QColor& color,
                                 const QString& title,
                                 const QString& text,
                                 QWidget* parent)
    : QPushButton(parent)
    , m_color(color)
    , m_title(title)
{
    setObjectName("PushButtonColor");
    setAutoFillBackground(true);
    setText(text);

    connect(this,SIGNAL(clicked()),this,SLOT(doWhenColorClicked()));

    display(color);
}


// Public method -------------------------------------------------------------//
void PushButtonColor::display(const QColor& color)
{
    m_color = color;
    QPushButton::setStyleSheet( QString("QPushButton#PushButtonColor{background-color: %1}").arg(color.name()) );
}

QColor PushButtonColor::color() const
{
    return m_color;
}


// Private slots -------------------------------------------------------------//
void PushButtonColor::doWhenColorClicked()
{
    emit beforeColorChanging();
    QColor memColor = m_color;

    QColorDialog colorDiag(this);
    connect(&colorDiag,SIGNAL(currentColorChanged(const QColor&)),
            this,SLOT(slotColorChanged(const QColor&)));
    colorDiag.setCurrentColor(memColor);
    colorDiag.exec();
    colorDiag.setWindowTitle( m_title );
    QColor color = colorDiag.selectedColor();

    if(color.isValid())
        m_color = color;
    else
        m_color =  memColor;

    QPushButton::setStyleSheet( QString("QPushButton#PushButtonColor{background-color: %1}").arg(m_color.name()) );
    emit colorChanged( m_color );
    emit colorHasChanged( m_color,text() );
}

void PushButtonColor::slotColorChanged(const QColor& color)
{
    if(color.isValid())
    {
        QPushButton::setStyleSheet( QString("QPushButton#PushButtonColor{background-color: %1}").arg(color.name()) );
        emit colorChanged( color );
    }
}
