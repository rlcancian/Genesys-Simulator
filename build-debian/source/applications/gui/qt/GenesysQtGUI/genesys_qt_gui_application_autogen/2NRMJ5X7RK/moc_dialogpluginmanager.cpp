/****************************************************************************
** Meta object code from reading C++ file 'dialogpluginmanager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../../source/applications/gui/qt/GenesysQtGUI/dialogs/dialogpluginmanager.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dialogpluginmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_DialogPluginManager_t {
    uint offsetsAndSizes[18];
    char stringdata0[20];
    char stringdata1[36];
    char stringdata2[1];
    char stringdata3[33];
    char stringdata4[27];
    char stringdata5[28];
    char stringdata6[37];
    char stringdata7[28];
    char stringdata8[29];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_DialogPluginManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_DialogPluginManager_t qt_meta_stringdata_DialogPluginManager = {
    {
        QT_MOC_LITERAL(0, 19),  // "DialogPluginManager"
        QT_MOC_LITERAL(20, 35),  // "on_pushButtonBrowseAutoload_c..."
        QT_MOC_LITERAL(56, 0),  // ""
        QT_MOC_LITERAL(57, 32),  // "on_pushButtonAutoLoadNow_clicked"
        QT_MOC_LITERAL(90, 26),  // "on_pushButtonCheck_clicked"
        QT_MOC_LITERAL(117, 27),  // "on_pushButtonInsert_clicked"
        QT_MOC_LITERAL(145, 36),  // "on_pushButtonResolveSelected_..."
        QT_MOC_LITERAL(182, 27),  // "on_pushButtonRemove_clicked"
        QT_MOC_LITERAL(210, 28)   // "on_pushButtonRefresh_clicked"
    },
    "DialogPluginManager",
    "on_pushButtonBrowseAutoload_clicked",
    "",
    "on_pushButtonAutoLoadNow_clicked",
    "on_pushButtonCheck_clicked",
    "on_pushButtonInsert_clicked",
    "on_pushButtonResolveSelected_clicked",
    "on_pushButtonRemove_clicked",
    "on_pushButtonRefresh_clicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_DialogPluginManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x08,    1 /* Private */,
       3,    0,   57,    2, 0x08,    2 /* Private */,
       4,    0,   58,    2, 0x08,    3 /* Private */,
       5,    0,   59,    2, 0x08,    4 /* Private */,
       6,    0,   60,    2, 0x08,    5 /* Private */,
       7,    0,   61,    2, 0x08,    6 /* Private */,
       8,    0,   62,    2, 0x08,    7 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject DialogPluginManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_DialogPluginManager.offsetsAndSizes,
    qt_meta_data_DialogPluginManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_DialogPluginManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DialogPluginManager, std::true_type>,
        // method 'on_pushButtonBrowseAutoload_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButtonAutoLoadNow_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButtonCheck_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButtonInsert_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButtonResolveSelected_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButtonRemove_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pushButtonRefresh_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void DialogPluginManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DialogPluginManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_pushButtonBrowseAutoload_clicked(); break;
        case 1: _t->on_pushButtonAutoLoadNow_clicked(); break;
        case 2: _t->on_pushButtonCheck_clicked(); break;
        case 3: _t->on_pushButtonInsert_clicked(); break;
        case 4: _t->on_pushButtonResolveSelected_clicked(); break;
        case 5: _t->on_pushButtonRemove_clicked(); break;
        case 6: _t->on_pushButtonRefresh_clicked(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *DialogPluginManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DialogPluginManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DialogPluginManager.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int DialogPluginManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
