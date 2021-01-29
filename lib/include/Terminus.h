#pragma once
#include "mctp.h"
#include "PdrRepository.h"
#include "clientNode.h"

class Terminus {
    public:
        int           deviceHandle;
        mctp_struct   mctpContext;
        clientNode    pldmEndpoint;
        PdrRepository localRepository;

        Terminus();
        ~Terminus();
};

