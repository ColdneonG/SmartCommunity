#ifndef FAMILYMEMBER_H
#define FAMILYMEMBER_H

#include <QDialog>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FamilyMember; }
QT_END_NAMESPACE

class FamilyMember : public QDialog
{
    Q_OBJECT

public:
    // 修改构造函数，接收DatabaseManager指针和parent参数
    FamilyMember(int ownerId, DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~FamilyMember() override;

    void setMemberData(int memberId, const QString &name, const QString &relation,
                       const QString &phone, const QString &idNumber);

protected:
    void accept() override;

private:
    Ui::FamilyMember *ui;
    int memberId;
    int ownerId;
    DatabaseManager *dbManager; // 修改为指针类型，接收外部传入的实例
};
#endif // FAMILYMEMBER_H
