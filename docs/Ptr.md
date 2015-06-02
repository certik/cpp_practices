# Performance Notes

In order for a class wrapper to produce equivalent assembly as the wrapped type
(e.g. a raw pointer, integer, ...), it must satisfy certain traits that in
general are dependent on the platform ABI.

Any class (no matter how complicated) will be optimized out in all cases,
except when passed as an argument to or returned from a function. The argument
passing conventions depend on the platform ABI, but in general, simple types
like raw pointers or integers as well as small classes of certain type will be
passed in registers (i.e. by value), while big classes on a stack (by
reference).

Example1: g++ 4.8.2, 4.9.2, 5.1.0 on 64bit will pass classes by value if and
only if the class is
[trivially
copyable](http://en.cppreference.com/w/cpp/concept/TriviallyCopyable) and its
size (measured using `sizeof`) is less or equal to 16 bytes.

Example2: icpc 15.0.1 on 64bit will pass by value if and only if the class is
[trivially
copyable](http://en.cppreference.com/w/cpp/concept/TriviallyCopyable) and size
less or equal to 16 bytes.

Other platforms might differ. Some authors propose to pass classes by value if
they are trivially copyable as well as a [standard layout
type](http://en.cppreference.com/w/cpp/concept/StandardLayoutType). So it is
probably a good idea to make it trivially copyable as well as a standard layout
type.

## Type Traits

If the class is [trivially
copyable](http://en.cppreference.com/w/cpp/concept/TriviallyCopyable)
([std::is_trivially_copyable](http://en.cppreference.com/w/cpp/types/is_trivially_copyable))
as well as has a [trivial default constructor](
http://en.cppreference.com/w/cpp/language/default_constructor#Trivial_default_constructor)
([std::is_trivially_default_constructible](http://en.cppreference.com/w/cpp/types/is_default_constructible))
then it is [trivial](http://en.cppreference.com/w/cpp/concept/TrivialType)
([std::is_trivial](http://en.cppreference.com/w/cpp/types/is_trivial)).  If in
addition it is a [standard layout
type](http://en.cppreference.com/w/cpp/concept/StandardLayoutType)
([std::is_standard_layout](http://en.cppreference.com/w/cpp/types/is_standard_layout)),
then it is a [POD](http://en.cppreference.com/w/cpp/concept/PODType)
([std::is_pod](http://en.cppreference.com/w/cpp/types/is_pod)).

# Example 1
Old:
```c++
int *a;
{
    int b=1;
    a = &b;
}
*a = 5; // Dangling, undefined behavior
```

New:
```c++
Ptr<int> a;
{
#ifdef DEBUG_MODE
    UniquePtr<int> b(new int(1));
#else
    int b=1;
#endif
#ifdef DEBUG_MODE
    a = b.ptr();
#else
    a = ptrFromRef(b);
#endif
}
*a = 5; // Dangling, throws an exception in Debug mode
```

# Example 2

Old:
```c++
class A {
private:
    std::map<int, int> m;
public:
    std::map<int, int> *get_access() {
        return &m;
    }
};
```

New:
```c++
class A {
private:
#ifdef DEBUG_MODE
    UniquePtr<std::map<int, int>> m;
#else
    std::map<int, int> m;
#endif
public:
    Ptr<std::map<int, int>> get_access() {
#ifdef DEBUG_MODE
        return m.ptr();
#else
        return ptrFromRef(m);
#endif
    }
};
```

# Example 3

Code ([original](https://github.com/ricochet-im/ricochet/blob/9f769bf872d4198e7456203c9ffd44963c47fa46/src/core/IdentityManager.cpp#L97])):
```c++
UserIdentity *IdentityManager::lookupHostname(const QString &hostname) const
{
    QString ohost = ContactIDValidator::hostnameFromID(hostname);
    if (ohost.isNull())
        ohost = hostname;

    if (!ohost.endsWith(QLatin1String(".onion")))
        ohost.append(QLatin1String(".onion"));

    for (QList<UserIdentity*>::ConstIterator it = m_identities.begin(); it != m_identities.end(); ++it)
    {
        if (ohost.compare((*it)->hostname(), Qt::CaseInsensitive) == 0)
            return *it;
    }

    return 0;
}
```

Code ([original](https://github.com/ricochet-im/ricochet/blob/9f769bf872d4198e7456203c9ffd44963c47fa46/src/core/ContactIDValidator.cpp#L65)):
```c++
ContactUser *ContactIDValidator::matchingContact(const QString &text) const
{
    ContactUser *u = 0;
    if (m_uniqueIdentity)
        u = m_uniqueIdentity->contacts.lookupHostname(text);
    return u;
}
```

And ([original](https://github.com/ricochet-im/ricochet/blob/954d6ed397fcde73f19564d3c2f7f3dfcda7996d/src/core/UserIdentity.cpp#L203)):
```c++
void UserIdentity::handleIncomingAuthedConnection(Connection *conn)
{
    if (conn->purpose() != Connection::Purpose::Unknown)
        return;

    QString clientName = conn->authenticatedIdentity(Connection::HiddenServiceAuth);
    if (clientName.isEmpty()) {
        BUG() << "Called to handle incoming authed connection without any authed name";
        return;
    }

    ContactUser *user = contacts.lookupHostname(clientName);
    if (!user) {
        // This client can start a contact request, for example. The purpose stays unknown, and the
        // connection will be killed if the purpose isn't changed before the timeout.
        qDebug() << "Have an incoming connection authenticated as unknown client" << clientName;
        return;
    }

    qDebug() << "Incoming connection authenticated as contact" << user->uniqueID << "with hostname" << clientName;
    user->assignConnection(conn);

    if (conn->parent() != user) {
        BUG() << "Connection wasn't claimed after authentication";
        conn->close();
    }
}
```
