#ifndef _PUSHBUTTONCOLOR_H_
#define _PUSHBUTTONCOLOR_H_

// Qt
#include <QColor>
#include <QPushButton>
#include <QString>


class PushButtonColor
        : public QPushButton
{
    Q_OBJECT

signals:
    void beforeColorChanging();
    void colorChanged(const QColor& color);
    void colorHasChanged(const QColor& color,const QString& text=QString());

public:
    // Constructor
    PushButtonColor(const QColor& color=Qt::gray,
                    const QString& title="",
                    const QString& text="",
                    QWidget* parent=0);
    // Destructor
    ~PushButtonColor(){}

    // Methods
    void display(const QColor& color);
    QColor color() const;

private slots:
    void doWhenColorClicked();
    void slotColorChanged(const QColor& color);

private:
    // Members
    QColor m_color;
    QString m_title;
};

#endif /* _PUSHBUTTONCOLOR_H_ */
