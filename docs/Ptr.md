# Example 1

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
