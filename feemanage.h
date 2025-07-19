#ifndef FEEMANAGE_H
#define FEEMANAGE_H

#include <QWidget>
#include "databasemanager.h"
#include <QTableWidget>
#include <QPushButton>


namespace Ui {
class FeeManage;
}

class FeeManage : public QWidget
{
    Q_OBJECT

public:
    explicit FeeManage(QWidget *parent = nullptr);
    ~FeeManage();

private:
    Ui::FeeManage *ui;
    DatabaseManager *m_dbManager;
    QTableWidget* tableWidget;
    QPushButton* saveBtn;
    QPushButton* exitBtn;
    void initTableWidget();
    void initButtons();
    void refreshData();
    void initConnections();
    void clearComboBoxes();
    void addEditComboBoxes();
    QMap<int,int> rowToItemIdMap;
    bool isEditing=0;
    int selectedItemId;

    bool initDatabaseConnection();
private slots:
    void onItemSelected();
    void onUpdateButtonClicked();
    void onExitButtonClicked();
    void onSaveButtonClicked();
};

#endif // FEEMANAGE_H
