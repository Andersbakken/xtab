/****************************************************************************
** Meta object code from reading C++ file 'GlobalShortcut.h'
**
** Created: Thu Mar 15 12:08:48 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "GlobalShortcut.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GlobalShortcut.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GlobalShortcut[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   16,   15,   15, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_GlobalShortcut[] = {
    "GlobalShortcut\0\0id\0activated(int)\0"
};

const QMetaObject GlobalShortcut::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_GlobalShortcut,
      qt_meta_data_GlobalShortcut, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GlobalShortcut::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GlobalShortcut::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GlobalShortcut::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GlobalShortcut))
        return static_cast<void*>(const_cast< GlobalShortcut*>(this));
    return QObject::qt_metacast(_clname);
}

int GlobalShortcut::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activated((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void GlobalShortcut::activated(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
