#-------------------------------------------------
#
# Project created by QtCreator 2025-07-06T14:25:57
#
#-------------------------------------------------

QT       += core gui sql widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

TARGET = SmartCommunity
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    owner_manage/estateregister.cpp \
    owner_manage/familymember.cpp \
    owner_manage/ownermodify.cpp \
    owner_manage/ownerquery.cpp \
    owner_manage/ownerregister.cpp \
    parking/parkingaccess.cpp \
    parking/parkingaccessquery.cpp \
    parking/parkingadd.cpp \
    parking/parkingapply.cpp \
    parking/parkinginformation.cpp \
    parking/parkingquery.cpp \
    parking/parkingrental.cpp \
    parking/parkingrentalmanage.cpp \
    parking/parkingshow.cpp \
    payment/feemanage.cpp \
    payment/feequery.cpp \
    payment/freequerystaff.cpp \
    personnel/addstaff.cpp \
    personnel/querystaff.cpp \
    repair/ownerrepairquery.cpp \
    repair/problemprocess.cpp \
    repair/problemreport.cpp \
    repair/repairevaluate.cpp \
    repair/staffrepairquery.cpp \
    adminhome.cpp \
    adminwindow.cpp \
    databasemanager.cpp \
    login.cpp \
    main.cpp \
    ownerhome.cpp \
    ownerwindow.cpp \
    staffhome.cpp \
    staffwindow.cpp \
    personnel/modifystaff.cpp \
    personnel/leaveapply.cpp \
    personnel/leaveapprove.cpp \
    personnel/attendancerecord.cpp \
    personnel/adminattendancequery.cpp \
    personnel/myattendancequery.cpp

HEADERS += \
    owner_manage/estateregister.h \
    owner_manage/familymember.h \
    owner_manage/ownermodify.h \
    owner_manage/ownerquery.h \
    owner_manage/ownerregister.h \
    parking/parkingaccess.h \
    parking/parkingaccessquery.h \
    parking/parkingadd.h \
    parking/parkingapply.h \
    parking/parkinginformation.h \
    parking/parkingquery.h \
    parking/parkingrental.h \
    parking/parkingrentalmanage.h \
    parking/parkingshow.h \
    payment/feemanage.h \
    payment/feequery.h \
    payment/freequerystaff.h \
    personnel/addstaff.h \
    personnel/querystaff.h \
    repair/ownerrepairquery.h \
    repair/problemprocess.h \
    repair/problemreport.h \
    repair/repairevaluate.h \
    repair/staffrepairquery.h \
    adminhome.h \
    adminwindow.h \
    databasemanager.h \
    login.h \
    ownerhome.h \
    ownerwindow.h \
    staffhome.h \
    staffwindow.h \
    personnel/modifystaff.h \
    personnel/leaveapply.h \
    personnel/leaveapprove.h \
    personnel/attendancerecord.h \
    personnel/adminattendancequery.h \
    personnel/myattendancequery.h

FORMS += \
    login.ui \
    adminwindow.ui \
    ownerwindow.ui \
    staffwindow.ui \
    repair/problemreport.ui \
    repair/problemprocess.ui \
    repair/repairevaluate.ui \
    repair/staffrepairquery.ui \
    repair/ownerrepairquery.ui \
    owner_manage/familymember.ui \
    owner_manage/ownerregister.ui \
    owner_manage/ownerquery.ui \
    owner_manage/ownermodify.ui \
    owner_manage/estateregister.ui \
    parking/parkingaccess.ui \
    parking/parkingaccessquery.ui \
    parking/parkingadd.ui \
    parking/parkingapply.ui \
    parking/parkinginformation.ui \
    parking/parkingquery.ui \
    parking/parkingrental.ui \
    parking/parkingrentalmanage.ui \
    parking/parkingshow.ui \
    staffhome.ui \
    adminhome.ui \
    ownerhome.ui \
    payment/feemanage.ui \
    payment/feequery.ui \
    payment/freequerystaff.ui \
    personnel/addstaff.ui \
    personnel/querystaff.ui \
    personnel/modifystaff.ui \
    personnel/leaveapply.ui \
    personnel/leaveapprove.ui \
    personnel/attendancerecord.ui \
    personnel/adminattendancequery.ui \
    personnel/myattendancequery.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qrc.qrc

SUBDIRS += \
