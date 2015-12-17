//
// Generated file, do not edit! Created by nedtool 4.6 from modules/messages/WaveShortMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "WaveShortMessage_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

Register_Class(WaveShortMessage);

WaveShortMessage::WaveShortMessage(const char *name, int kind) : ::cPacket(name,kind)
{
    this->wsmVersion_var = 0;
    this->securityType_var = 0;
    this->channelNumber_var = 0;
    this->dataRate_var = 1;
    this->priority_var = 3;
    this->psid_var = 0;
    this->psc_var = "Service with some Data";
    this->wsmLength_var = 0;
    this->wsmData_var = "Some Data";
    this->serial_var = 0;
    this->timestamp_var = 0;
    this->senderAddress_var = 0;
    this->recipientAddress_var = 268435455;
    this->source_var = 0;
    this->target_var = 0;
    this->summaryVector_var = false;
    this->requestMessages_var = false;
    this->globalMessageIdentificaton_var = 0;
    this->localMessageIdentificaton_var = 0;
    this->hopCount_var = 0;
    this->ackRequest_var = false;
    this->roadId_var = "";
    this->senderSpeed_var = 0.0;
    this->category_var = 0;
    this->vehicleId_var = 0;
    this->heading_var = 0;
    this->messageTimestampGenerate_var = 0;
}

WaveShortMessage::WaveShortMessage(const WaveShortMessage& other) : ::cPacket(other)
{
    copy(other);
}

WaveShortMessage::~WaveShortMessage()
{
}

WaveShortMessage& WaveShortMessage::operator=(const WaveShortMessage& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void WaveShortMessage::copy(const WaveShortMessage& other)
{
    this->wsmVersion_var = other.wsmVersion_var;
    this->securityType_var = other.securityType_var;
    this->channelNumber_var = other.channelNumber_var;
    this->dataRate_var = other.dataRate_var;
    this->priority_var = other.priority_var;
    this->psid_var = other.psid_var;
    this->psc_var = other.psc_var;
    this->wsmLength_var = other.wsmLength_var;
    this->wsmData_var = other.wsmData_var;
    this->serial_var = other.serial_var;
    this->senderPos_var = other.senderPos_var;
    this->timestamp_var = other.timestamp_var;
    this->senderAddress_var = other.senderAddress_var;
    this->recipientAddress_var = other.recipientAddress_var;
    this->source_var = other.source_var;
    this->target_var = other.target_var;
    this->summaryVector_var = other.summaryVector_var;
    this->requestMessages_var = other.requestMessages_var;
    this->globalMessageIdentificaton_var = other.globalMessageIdentificaton_var;
    this->localMessageIdentificaton_var = other.localMessageIdentificaton_var;
    this->hopCount_var = other.hopCount_var;
    this->ackRequest_var = other.ackRequest_var;
    this->roadId_var = other.roadId_var;
    this->senderSpeed_var = other.senderSpeed_var;
    this->category_var = other.category_var;
    this->vehicleId_var = other.vehicleId_var;
    this->heading_var = other.heading_var;
    this->TargetPos_var = other.TargetPos_var;
    this->senderPosBack_var = other.senderPosBack_var;
    this->messageTimestampGenerate_var = other.messageTimestampGenerate_var;
}

void WaveShortMessage::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->wsmVersion_var);
    doPacking(b,this->securityType_var);
    doPacking(b,this->channelNumber_var);
    doPacking(b,this->dataRate_var);
    doPacking(b,this->priority_var);
    doPacking(b,this->psid_var);
    doPacking(b,this->psc_var);
    doPacking(b,this->wsmLength_var);
    doPacking(b,this->wsmData_var);
    doPacking(b,this->serial_var);
    doPacking(b,this->senderPos_var);
    doPacking(b,this->timestamp_var);
    doPacking(b,this->senderAddress_var);
    doPacking(b,this->recipientAddress_var);
    doPacking(b,this->source_var);
    doPacking(b,this->target_var);
    doPacking(b,this->summaryVector_var);
    doPacking(b,this->requestMessages_var);
    doPacking(b,this->globalMessageIdentificaton_var);
    doPacking(b,this->localMessageIdentificaton_var);
    doPacking(b,this->hopCount_var);
    doPacking(b,this->ackRequest_var);
    doPacking(b,this->roadId_var);
    doPacking(b,this->senderSpeed_var);
    doPacking(b,this->category_var);
    doPacking(b,this->vehicleId_var);
    doPacking(b,this->heading_var);
    doPacking(b,this->TargetPos_var);
    doPacking(b,this->senderPosBack_var);
    doPacking(b,this->messageTimestampGenerate_var);
}

void WaveShortMessage::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->wsmVersion_var);
    doUnpacking(b,this->securityType_var);
    doUnpacking(b,this->channelNumber_var);
    doUnpacking(b,this->dataRate_var);
    doUnpacking(b,this->priority_var);
    doUnpacking(b,this->psid_var);
    doUnpacking(b,this->psc_var);
    doUnpacking(b,this->wsmLength_var);
    doUnpacking(b,this->wsmData_var);
    doUnpacking(b,this->serial_var);
    doUnpacking(b,this->senderPos_var);
    doUnpacking(b,this->timestamp_var);
    doUnpacking(b,this->senderAddress_var);
    doUnpacking(b,this->recipientAddress_var);
    doUnpacking(b,this->source_var);
    doUnpacking(b,this->target_var);
    doUnpacking(b,this->summaryVector_var);
    doUnpacking(b,this->requestMessages_var);
    doUnpacking(b,this->globalMessageIdentificaton_var);
    doUnpacking(b,this->localMessageIdentificaton_var);
    doUnpacking(b,this->hopCount_var);
    doUnpacking(b,this->ackRequest_var);
    doUnpacking(b,this->roadId_var);
    doUnpacking(b,this->senderSpeed_var);
    doUnpacking(b,this->category_var);
    doUnpacking(b,this->vehicleId_var);
    doUnpacking(b,this->heading_var);
    doUnpacking(b,this->TargetPos_var);
    doUnpacking(b,this->senderPosBack_var);
    doUnpacking(b,this->messageTimestampGenerate_var);
}

int WaveShortMessage::getWsmVersion() const
{
    return wsmVersion_var;
}

void WaveShortMessage::setWsmVersion(int wsmVersion)
{
    this->wsmVersion_var = wsmVersion;
}

int WaveShortMessage::getSecurityType() const
{
    return securityType_var;
}

void WaveShortMessage::setSecurityType(int securityType)
{
    this->securityType_var = securityType;
}

int WaveShortMessage::getChannelNumber() const
{
    return channelNumber_var;
}

void WaveShortMessage::setChannelNumber(int channelNumber)
{
    this->channelNumber_var = channelNumber;
}

int WaveShortMessage::getDataRate() const
{
    return dataRate_var;
}

void WaveShortMessage::setDataRate(int dataRate)
{
    this->dataRate_var = dataRate;
}

int WaveShortMessage::getPriority() const
{
    return priority_var;
}

void WaveShortMessage::setPriority(int priority)
{
    this->priority_var = priority;
}

int WaveShortMessage::getPsid() const
{
    return psid_var;
}

void WaveShortMessage::setPsid(int psid)
{
    this->psid_var = psid;
}

const char * WaveShortMessage::getPsc() const
{
    return psc_var.c_str();
}

void WaveShortMessage::setPsc(const char * psc)
{
    this->psc_var = psc;
}

int WaveShortMessage::getWsmLength() const
{
    return wsmLength_var;
}

void WaveShortMessage::setWsmLength(int wsmLength)
{
    this->wsmLength_var = wsmLength;
}

const char * WaveShortMessage::getWsmData() const
{
    return wsmData_var.c_str();
}

void WaveShortMessage::setWsmData(const char * wsmData)
{
    this->wsmData_var = wsmData;
}

int WaveShortMessage::getSerial() const
{
    return serial_var;
}

void WaveShortMessage::setSerial(int serial)
{
    this->serial_var = serial;
}

Coord& WaveShortMessage::getSenderPos()
{
    return senderPos_var;
}

void WaveShortMessage::setSenderPos(const Coord& senderPos)
{
    this->senderPos_var = senderPos;
}

simtime_t WaveShortMessage::getTimestamp() const
{
    return timestamp_var;
}

void WaveShortMessage::setTimestamp(simtime_t timestamp)
{
    this->timestamp_var = timestamp;
}

unsigned int WaveShortMessage::getSenderAddress() const
{
    return senderAddress_var;
}

void WaveShortMessage::setSenderAddress(unsigned int senderAddress)
{
    this->senderAddress_var = senderAddress;
}

unsigned int WaveShortMessage::getRecipientAddress() const
{
    return recipientAddress_var;
}

void WaveShortMessage::setRecipientAddress(unsigned int recipientAddress)
{
    this->recipientAddress_var = recipientAddress;
}

const char * WaveShortMessage::getSource() const
{
    return source_var.c_str();
}

void WaveShortMessage::setSource(const char * source)
{
    this->source_var = source;
}

const char * WaveShortMessage::getTarget() const
{
    return target_var.c_str();
}

void WaveShortMessage::setTarget(const char * target)
{
    this->target_var = target;
}

bool WaveShortMessage::getSummaryVector() const
{
    return summaryVector_var;
}

void WaveShortMessage::setSummaryVector(bool summaryVector)
{
    this->summaryVector_var = summaryVector;
}

bool WaveShortMessage::getRequestMessages() const
{
    return requestMessages_var;
}

void WaveShortMessage::setRequestMessages(bool requestMessages)
{
    this->requestMessages_var = requestMessages;
}

const char * WaveShortMessage::getGlobalMessageIdentificaton() const
{
    return globalMessageIdentificaton_var.c_str();
}

void WaveShortMessage::setGlobalMessageIdentificaton(const char * globalMessageIdentificaton)
{
    this->globalMessageIdentificaton_var = globalMessageIdentificaton;
}

const char * WaveShortMessage::getLocalMessageIdentificaton() const
{
    return localMessageIdentificaton_var.c_str();
}

void WaveShortMessage::setLocalMessageIdentificaton(const char * localMessageIdentificaton)
{
    this->localMessageIdentificaton_var = localMessageIdentificaton;
}

unsigned int WaveShortMessage::getHopCount() const
{
    return hopCount_var;
}

void WaveShortMessage::setHopCount(unsigned int hopCount)
{
    this->hopCount_var = hopCount;
}

bool WaveShortMessage::getAckRequest() const
{
    return ackRequest_var;
}

void WaveShortMessage::setAckRequest(bool ackRequest)
{
    this->ackRequest_var = ackRequest;
}

const char * WaveShortMessage::getRoadId() const
{
    return roadId_var.c_str();
}

void WaveShortMessage::setRoadId(const char * roadId)
{
    this->roadId_var = roadId;
}

double WaveShortMessage::getSenderSpeed() const
{
    return senderSpeed_var;
}

void WaveShortMessage::setSenderSpeed(double senderSpeed)
{
    this->senderSpeed_var = senderSpeed;
}

unsigned short WaveShortMessage::getCategory() const
{
    return category_var;
}

void WaveShortMessage::setCategory(unsigned short category)
{
    this->category_var = category;
}

int WaveShortMessage::getVehicleId() const
{
    return vehicleId_var;
}

void WaveShortMessage::setVehicleId(int vehicleId)
{
    this->vehicleId_var = vehicleId;
}

unsigned short WaveShortMessage::getHeading() const
{
    return heading_var;
}

void WaveShortMessage::setHeading(unsigned short heading)
{
    this->heading_var = heading;
}

Coord& WaveShortMessage::getTargetPos()
{
    return TargetPos_var;
}

void WaveShortMessage::setTargetPos(const Coord& TargetPos)
{
    this->TargetPos_var = TargetPos;
}

Coord& WaveShortMessage::getSenderPosBack()
{
    return senderPosBack_var;
}

void WaveShortMessage::setSenderPosBack(const Coord& senderPosBack)
{
    this->senderPosBack_var = senderPosBack;
}

simtime_t WaveShortMessage::getMessageTimestampGenerate() const
{
    return messageTimestampGenerate_var;
}

void WaveShortMessage::setMessageTimestampGenerate(simtime_t messageTimestampGenerate)
{
    this->messageTimestampGenerate_var = messageTimestampGenerate;
}

class WaveShortMessageDescriptor : public cClassDescriptor
{
  public:
    WaveShortMessageDescriptor();
    virtual ~WaveShortMessageDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(WaveShortMessageDescriptor);

WaveShortMessageDescriptor::WaveShortMessageDescriptor() : cClassDescriptor("WaveShortMessage", "cPacket")
{
}

WaveShortMessageDescriptor::~WaveShortMessageDescriptor()
{
}

bool WaveShortMessageDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<WaveShortMessage *>(obj)!=NULL;
}

const char *WaveShortMessageDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int WaveShortMessageDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 30+basedesc->getFieldCount(object) : 30;
}

unsigned int WaveShortMessageDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<30) ? fieldTypeFlags[field] : 0;
}

const char *WaveShortMessageDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "wsmVersion",
        "securityType",
        "channelNumber",
        "dataRate",
        "priority",
        "psid",
        "psc",
        "wsmLength",
        "wsmData",
        "serial",
        "senderPos",
        "timestamp",
        "senderAddress",
        "recipientAddress",
        "source",
        "target",
        "summaryVector",
        "requestMessages",
        "globalMessageIdentificaton",
        "localMessageIdentificaton",
        "hopCount",
        "ackRequest",
        "roadId",
        "senderSpeed",
        "category",
        "vehicleId",
        "heading",
        "TargetPos",
        "senderPosBack",
        "messageTimestampGenerate",
    };
    return (field>=0 && field<30) ? fieldNames[field] : NULL;
}

int WaveShortMessageDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='w' && strcmp(fieldName, "wsmVersion")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "securityType")==0) return base+1;
    if (fieldName[0]=='c' && strcmp(fieldName, "channelNumber")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "dataRate")==0) return base+3;
    if (fieldName[0]=='p' && strcmp(fieldName, "priority")==0) return base+4;
    if (fieldName[0]=='p' && strcmp(fieldName, "psid")==0) return base+5;
    if (fieldName[0]=='p' && strcmp(fieldName, "psc")==0) return base+6;
    if (fieldName[0]=='w' && strcmp(fieldName, "wsmLength")==0) return base+7;
    if (fieldName[0]=='w' && strcmp(fieldName, "wsmData")==0) return base+8;
    if (fieldName[0]=='s' && strcmp(fieldName, "serial")==0) return base+9;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderPos")==0) return base+10;
    if (fieldName[0]=='t' && strcmp(fieldName, "timestamp")==0) return base+11;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderAddress")==0) return base+12;
    if (fieldName[0]=='r' && strcmp(fieldName, "recipientAddress")==0) return base+13;
    if (fieldName[0]=='s' && strcmp(fieldName, "source")==0) return base+14;
    if (fieldName[0]=='t' && strcmp(fieldName, "target")==0) return base+15;
    if (fieldName[0]=='s' && strcmp(fieldName, "summaryVector")==0) return base+16;
    if (fieldName[0]=='r' && strcmp(fieldName, "requestMessages")==0) return base+17;
    if (fieldName[0]=='g' && strcmp(fieldName, "globalMessageIdentificaton")==0) return base+18;
    if (fieldName[0]=='l' && strcmp(fieldName, "localMessageIdentificaton")==0) return base+19;
    if (fieldName[0]=='h' && strcmp(fieldName, "hopCount")==0) return base+20;
    if (fieldName[0]=='a' && strcmp(fieldName, "ackRequest")==0) return base+21;
    if (fieldName[0]=='r' && strcmp(fieldName, "roadId")==0) return base+22;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderSpeed")==0) return base+23;
    if (fieldName[0]=='c' && strcmp(fieldName, "category")==0) return base+24;
    if (fieldName[0]=='v' && strcmp(fieldName, "vehicleId")==0) return base+25;
    if (fieldName[0]=='h' && strcmp(fieldName, "heading")==0) return base+26;
    if (fieldName[0]=='T' && strcmp(fieldName, "TargetPos")==0) return base+27;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderPosBack")==0) return base+28;
    if (fieldName[0]=='m' && strcmp(fieldName, "messageTimestampGenerate")==0) return base+29;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *WaveShortMessageDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "string",
        "int",
        "string",
        "int",
        "Coord",
        "simtime_t",
        "unsigned int",
        "unsigned int",
        "string",
        "string",
        "bool",
        "bool",
        "string",
        "string",
        "unsigned int",
        "bool",
        "string",
        "double",
        "unsigned short",
        "int",
        "unsigned short",
        "Coord",
        "Coord",
        "simtime_t",
    };
    return (field>=0 && field<30) ? fieldTypeStrings[field] : NULL;
}

const char *WaveShortMessageDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int WaveShortMessageDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string WaveShortMessageDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getWsmVersion());
        case 1: return long2string(pp->getSecurityType());
        case 2: return long2string(pp->getChannelNumber());
        case 3: return long2string(pp->getDataRate());
        case 4: return long2string(pp->getPriority());
        case 5: return long2string(pp->getPsid());
        case 6: return oppstring2string(pp->getPsc());
        case 7: return long2string(pp->getWsmLength());
        case 8: return oppstring2string(pp->getWsmData());
        case 9: return long2string(pp->getSerial());
        case 10: {std::stringstream out; out << pp->getSenderPos(); return out.str();}
        case 11: return double2string(pp->getTimestamp());
        case 12: return ulong2string(pp->getSenderAddress());
        case 13: return ulong2string(pp->getRecipientAddress());
        case 14: return oppstring2string(pp->getSource());
        case 15: return oppstring2string(pp->getTarget());
        case 16: return bool2string(pp->getSummaryVector());
        case 17: return bool2string(pp->getRequestMessages());
        case 18: return oppstring2string(pp->getGlobalMessageIdentificaton());
        case 19: return oppstring2string(pp->getLocalMessageIdentificaton());
        case 20: return ulong2string(pp->getHopCount());
        case 21: return bool2string(pp->getAckRequest());
        case 22: return oppstring2string(pp->getRoadId());
        case 23: return double2string(pp->getSenderSpeed());
        case 24: return ulong2string(pp->getCategory());
        case 25: return long2string(pp->getVehicleId());
        case 26: return ulong2string(pp->getHeading());
        case 27: {std::stringstream out; out << pp->getTargetPos(); return out.str();}
        case 28: {std::stringstream out; out << pp->getSenderPosBack(); return out.str();}
        case 29: return double2string(pp->getMessageTimestampGenerate());
        default: return "";
    }
}

bool WaveShortMessageDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        case 0: pp->setWsmVersion(string2long(value)); return true;
        case 1: pp->setSecurityType(string2long(value)); return true;
        case 2: pp->setChannelNumber(string2long(value)); return true;
        case 3: pp->setDataRate(string2long(value)); return true;
        case 4: pp->setPriority(string2long(value)); return true;
        case 5: pp->setPsid(string2long(value)); return true;
        case 6: pp->setPsc((value)); return true;
        case 7: pp->setWsmLength(string2long(value)); return true;
        case 8: pp->setWsmData((value)); return true;
        case 9: pp->setSerial(string2long(value)); return true;
        case 11: pp->setTimestamp(string2double(value)); return true;
        case 12: pp->setSenderAddress(string2ulong(value)); return true;
        case 13: pp->setRecipientAddress(string2ulong(value)); return true;
        case 14: pp->setSource((value)); return true;
        case 15: pp->setTarget((value)); return true;
        case 16: pp->setSummaryVector(string2bool(value)); return true;
        case 17: pp->setRequestMessages(string2bool(value)); return true;
        case 18: pp->setGlobalMessageIdentificaton((value)); return true;
        case 19: pp->setLocalMessageIdentificaton((value)); return true;
        case 20: pp->setHopCount(string2ulong(value)); return true;
        case 21: pp->setAckRequest(string2bool(value)); return true;
        case 22: pp->setRoadId((value)); return true;
        case 23: pp->setSenderSpeed(string2double(value)); return true;
        case 24: pp->setCategory(string2ulong(value)); return true;
        case 25: pp->setVehicleId(string2long(value)); return true;
        case 26: pp->setHeading(string2ulong(value)); return true;
        case 29: pp->setMessageTimestampGenerate(string2double(value)); return true;
        default: return false;
    }
}

const char *WaveShortMessageDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 10: return opp_typename(typeid(Coord));
        case 27: return opp_typename(typeid(Coord));
        case 28: return opp_typename(typeid(Coord));
        default: return NULL;
    };
}

void *WaveShortMessageDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    WaveShortMessage *pp = (WaveShortMessage *)object; (void)pp;
    switch (field) {
        case 10: return (void *)(&pp->getSenderPos()); break;
        case 27: return (void *)(&pp->getTargetPos()); break;
        case 28: return (void *)(&pp->getSenderPosBack()); break;
        default: return NULL;
    }
}


