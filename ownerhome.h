#ifndef OWNERHOME_H
#define OWNERHOME_H

#include <QWidget>
#include <QVariantList>
#include <QListWidgetItem>
#include "databasemanager.h"

namespace Ui {
class OwnerHome;
}

class OwnerHome : public QWidget
{
    Q_OBJECT

public:
    explicit OwnerHome(int ownerID, QWidget *parent = nullptr);
    ~OwnerHome();

private:
    Ui::OwnerHome *ui;
    int m_ownerID;
    DatabaseManager *m_dbManager;
    int m_selectedNoticeId;

    bool initDatabase();
    void setStyle();
    void loadImportantInfo();
    void refreshNoticeList();

private slots:
    void on_noticeList_itemClicked(QListWidgetItem *item);
};

#endif // OWNERHOME_H
