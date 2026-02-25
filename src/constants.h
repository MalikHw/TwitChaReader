#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

#ifdef TWITCH_CLIENT_ID
    static const QString TWITCH_APP_CLIENT_ID = TWITCH_CLIENT_ID;
#else
    static const QString TWITCH_APP_CLIENT_ID = "kimne78kx3ncx6brgo4mv6wki5h1ko";
#endif

#endif
