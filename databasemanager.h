#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVector>
#include <QVariant>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:  // 构造函数改为公有，允许外部实例化
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;  // 显式声明析构函数

    // 禁用拷贝
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initDatabase(const QString &connectionName);  //手动调用初始化数据库
    bool isOpen() const;
    QString lastError() const;
    bool executeQuery(const QString& queryString, const QVariantList& bindValues = QVariantList());
    void closeDatabase(); // 关闭数据库连接

    QVector<QVariantMap> executeSelectQuery(const QString& queryString, const QVariantList& bindValues = QVariantList());
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

signals:
    void databaseOpened(bool success, const QString& message);
    void queryExecuted(bool success, const QString& message);

private:  // 成员变量保持不变
    QSqlDatabase m_database;
    bool m_isOpen;
};
#endif // DATABASEMANAGER_H
