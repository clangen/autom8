package org.clangen.autom8.service;

import org.clangen.autom8.net.Message;

interface IClientService {
    void sendMessage(in org.clangen.autom8.net.Message message);
    void reconnect();
    void disconnect();
    int getState();
    long getConnectionId();
}