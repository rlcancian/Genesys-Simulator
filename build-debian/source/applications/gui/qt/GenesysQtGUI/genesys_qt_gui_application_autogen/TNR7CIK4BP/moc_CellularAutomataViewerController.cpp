/****************************************************************************
** Meta object code from reading C++ file 'CellularAutomataViewerController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../../source/applications/gui/qt/GenesysQtGUI/cellular_automata/CellularAutomataViewerController.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CellularAutomataViewerController.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CellularAutomataViewerController_t {
    uint offsetsAndSizes[24];
    char stringdata0[33];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[12];
    char stringdata5[5];
    char stringdata6[15];
    char stringdata7[8];
    char stringdata8[18];
    char stringdata9[14];
    char stringdata10[8];
    char stringdata11[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CellularAutomataViewerController_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CellularAutomataViewerController_t qt_meta_stringdata_CellularAutomataViewerController = {
    {
        QT_MOC_LITERAL(0, 32),  // "CellularAutomataViewerController"
        QT_MOC_LITERAL(33, 12),  // "modelChanged"
        QT_MOC_LITERAL(46, 0),  // ""
        QT_MOC_LITERAL(47, 15),  // "settingsChanged"
        QT_MOC_LITERAL(63, 11),  // "timeChanged"
        QT_MOC_LITERAL(75, 4),  // "time"
        QT_MOC_LITERAL(80, 14),  // "runningChanged"
        QT_MOC_LITERAL(95, 7),  // "running"
        QT_MOC_LITERAL(103, 17),  // "paintStateChanged"
        QT_MOC_LITERAL(121, 13),  // "statusMessage"
        QT_MOC_LITERAL(135, 7),  // "message"
        QT_MOC_LITERAL(143, 15)   // "_onTimerTimeout"
    },
    "CellularAutomataViewerController",
    "modelChanged",
    "",
    "settingsChanged",
    "timeChanged",
    "time",
    "runningChanged",
    "running",
    "paintStateChanged",
    "statusMessage",
    "message",
    "_onTimerTimeout"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CellularAutomataViewerController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    1 /* Public */,
       3,    0,   57,    2, 0x06,    2 /* Public */,
       4,    1,   58,    2, 0x06,    3 /* Public */,
       6,    1,   61,    2, 0x06,    5 /* Public */,
       8,    0,   64,    2, 0x06,    7 /* Public */,
       9,    1,   65,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,   68,    2, 0x08,   10 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt,    5,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject CellularAutomataViewerController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CellularAutomataViewerController.offsetsAndSizes,
    qt_meta_data_CellularAutomataViewerController,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CellularAutomataViewerController_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CellularAutomataViewerController, std::true_type>,
        // method 'modelChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'settingsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'timeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<unsigned int, std::false_type>,
        // method 'runningChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'paintStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'statusMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method '_onTimerTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CellularAutomataViewerController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CellularAutomataViewerController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->modelChanged(); break;
        case 1: _t->settingsChanged(); break;
        case 2: _t->timeChanged((*reinterpret_cast< std::add_pointer_t<uint>>(_a[1]))); break;
        case 3: _t->runningChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->paintStateChanged(); break;
        case 5: _t->statusMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->_onTimerTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CellularAutomataViewerController::*)();
            if (_t _q_method = &CellularAutomataViewerController::modelChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CellularAutomataViewerController::*)();
            if (_t _q_method = &CellularAutomataViewerController::settingsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CellularAutomataViewerController::*)(unsigned int );
            if (_t _q_method = &CellularAutomataViewerController::timeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CellularAutomataViewerController::*)(bool );
            if (_t _q_method = &CellularAutomataViewerController::runningChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CellularAutomataViewerController::*)();
            if (_t _q_method = &CellularAutomataViewerController::paintStateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (CellularAutomataViewerController::*)(const QString & );
            if (_t _q_method = &CellularAutomataViewerController::statusMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *CellularAutomataViewerController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CellularAutomataViewerController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CellularAutomataViewerController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CellularAutomataViewerController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void CellularAutomataViewerController::modelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CellularAutomataViewerController::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CellularAutomataViewerController::timeChanged(unsigned int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CellularAutomataViewerController::runningChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void CellularAutomataViewerController::paintStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void CellularAutomataViewerController::statusMessage(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
