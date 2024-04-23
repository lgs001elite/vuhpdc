//
// Generated file, do not edit! Created by nedtool 5.6 from src/dataFrame.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "dataFrame_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
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

Register_Class(metaFrame)

metaFrame::metaFrame(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->findCount = 0;
    this->hop2Sink = 0;
    this->actualHop = 0;
    this->destinationId = 0;
    this->nextHopId = 0;
    this->sourceId = 0;
    this->senderId = 0;
    this->msgType = 0;
    this->msgID = 0;
    this->msgMark = 0;
    this->pairMark = 0;
    this->roundMark = 0;
    this->chargeSlot = 0;
    this->endTrans = 0;
    this->sendTime = 0;
}

metaFrame::metaFrame(const metaFrame& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

metaFrame::~metaFrame()
{
}

metaFrame& metaFrame::operator=(const metaFrame& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void metaFrame::copy(const metaFrame& other)
{
    this->findCount = other.findCount;
    this->hop2Sink = other.hop2Sink;
    this->actualHop = other.actualHop;
    this->destinationId = other.destinationId;
    this->nextHopId = other.nextHopId;
    this->sourceId = other.sourceId;
    this->senderId = other.senderId;
    this->msgType = other.msgType;
    this->msgID = other.msgID;
    this->msgMark = other.msgMark;
    this->pairMark = other.pairMark;
    this->roundMark = other.roundMark;
    this->chargeSlot = other.chargeSlot;
    this->endTrans = other.endTrans;
    this->sendTime = other.sendTime;
}

void metaFrame::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->findCount);
    doParsimPacking(b,this->hop2Sink);
    doParsimPacking(b,this->actualHop);
    doParsimPacking(b,this->destinationId);
    doParsimPacking(b,this->nextHopId);
    doParsimPacking(b,this->sourceId);
    doParsimPacking(b,this->senderId);
    doParsimPacking(b,this->msgType);
    doParsimPacking(b,this->msgID);
    doParsimPacking(b,this->msgMark);
    doParsimPacking(b,this->pairMark);
    doParsimPacking(b,this->roundMark);
    doParsimPacking(b,this->chargeSlot);
    doParsimPacking(b,this->endTrans);
    doParsimPacking(b,this->sendTime);
}

void metaFrame::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->findCount);
    doParsimUnpacking(b,this->hop2Sink);
    doParsimUnpacking(b,this->actualHop);
    doParsimUnpacking(b,this->destinationId);
    doParsimUnpacking(b,this->nextHopId);
    doParsimUnpacking(b,this->sourceId);
    doParsimUnpacking(b,this->senderId);
    doParsimUnpacking(b,this->msgType);
    doParsimUnpacking(b,this->msgID);
    doParsimUnpacking(b,this->msgMark);
    doParsimUnpacking(b,this->pairMark);
    doParsimUnpacking(b,this->roundMark);
    doParsimUnpacking(b,this->chargeSlot);
    doParsimUnpacking(b,this->endTrans);
    doParsimUnpacking(b,this->sendTime);
}

int metaFrame::getFindCount() const
{
    return this->findCount;
}

void metaFrame::setFindCount(int findCount)
{
    this->findCount = findCount;
}

int metaFrame::getHop2Sink() const
{
    return this->hop2Sink;
}

void metaFrame::setHop2Sink(int hop2Sink)
{
    this->hop2Sink = hop2Sink;
}

int metaFrame::getActualHop() const
{
    return this->actualHop;
}

void metaFrame::setActualHop(int actualHop)
{
    this->actualHop = actualHop;
}

int metaFrame::getDestinationId() const
{
    return this->destinationId;
}

void metaFrame::setDestinationId(int destinationId)
{
    this->destinationId = destinationId;
}

int metaFrame::getNextHopId() const
{
    return this->nextHopId;
}

void metaFrame::setNextHopId(int nextHopId)
{
    this->nextHopId = nextHopId;
}

int metaFrame::getSourceId() const
{
    return this->sourceId;
}

void metaFrame::setSourceId(int sourceId)
{
    this->sourceId = sourceId;
}

int metaFrame::getSenderId() const
{
    return this->senderId;
}

void metaFrame::setSenderId(int senderId)
{
    this->senderId = senderId;
}

int metaFrame::getMsgType() const
{
    return this->msgType;
}

void metaFrame::setMsgType(int msgType)
{
    this->msgType = msgType;
}

int metaFrame::getMsgID() const
{
    return this->msgID;
}

void metaFrame::setMsgID(int msgID)
{
    this->msgID = msgID;
}

int metaFrame::getMsgMark() const
{
    return this->msgMark;
}

void metaFrame::setMsgMark(int msgMark)
{
    this->msgMark = msgMark;
}

int metaFrame::getPairMark() const
{
    return this->pairMark;
}

void metaFrame::setPairMark(int pairMark)
{
    this->pairMark = pairMark;
}

int metaFrame::getRoundMark() const
{
    return this->roundMark;
}

void metaFrame::setRoundMark(int roundMark)
{
    this->roundMark = roundMark;
}

int metaFrame::getChargeSlot() const
{
    return this->chargeSlot;
}

void metaFrame::setChargeSlot(int chargeSlot)
{
    this->chargeSlot = chargeSlot;
}

int metaFrame::getEndTrans() const
{
    return this->endTrans;
}

void metaFrame::setEndTrans(int endTrans)
{
    this->endTrans = endTrans;
}

::omnetpp::simtime_t metaFrame::getSendTime() const
{
    return this->sendTime;
}

void metaFrame::setSendTime(::omnetpp::simtime_t sendTime)
{
    this->sendTime = sendTime;
}

class metaFrameDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    metaFrameDescriptor();
    virtual ~metaFrameDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(metaFrameDescriptor)

metaFrameDescriptor::metaFrameDescriptor() : omnetpp::cClassDescriptor("metaFrame", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

metaFrameDescriptor::~metaFrameDescriptor()
{
    delete[] propertynames;
}

bool metaFrameDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<metaFrame *>(obj)!=nullptr;
}

const char **metaFrameDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *metaFrameDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int metaFrameDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 15+basedesc->getFieldCount() : 15;
}

unsigned int metaFrameDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
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
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<15) ? fieldTypeFlags[field] : 0;
}

const char *metaFrameDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "findCount",
        "hop2Sink",
        "actualHop",
        "destinationId",
        "nextHopId",
        "sourceId",
        "senderId",
        "msgType",
        "msgID",
        "msgMark",
        "pairMark",
        "roundMark",
        "chargeSlot",
        "endTrans",
        "sendTime",
    };
    return (field>=0 && field<15) ? fieldNames[field] : nullptr;
}

int metaFrameDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='f' && strcmp(fieldName, "findCount")==0) return base+0;
    if (fieldName[0]=='h' && strcmp(fieldName, "hop2Sink")==0) return base+1;
    if (fieldName[0]=='a' && strcmp(fieldName, "actualHop")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "destinationId")==0) return base+3;
    if (fieldName[0]=='n' && strcmp(fieldName, "nextHopId")==0) return base+4;
    if (fieldName[0]=='s' && strcmp(fieldName, "sourceId")==0) return base+5;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderId")==0) return base+6;
    if (fieldName[0]=='m' && strcmp(fieldName, "msgType")==0) return base+7;
    if (fieldName[0]=='m' && strcmp(fieldName, "msgID")==0) return base+8;
    if (fieldName[0]=='m' && strcmp(fieldName, "msgMark")==0) return base+9;
    if (fieldName[0]=='p' && strcmp(fieldName, "pairMark")==0) return base+10;
    if (fieldName[0]=='r' && strcmp(fieldName, "roundMark")==0) return base+11;
    if (fieldName[0]=='c' && strcmp(fieldName, "chargeSlot")==0) return base+12;
    if (fieldName[0]=='e' && strcmp(fieldName, "endTrans")==0) return base+13;
    if (fieldName[0]=='s' && strcmp(fieldName, "sendTime")==0) return base+14;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *metaFrameDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "simtime_t",
    };
    return (field>=0 && field<15) ? fieldTypeStrings[field] : nullptr;
}

const char **metaFrameDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *metaFrameDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int metaFrameDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    metaFrame *pp = (metaFrame *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *metaFrameDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    metaFrame *pp = (metaFrame *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string metaFrameDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    metaFrame *pp = (metaFrame *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getFindCount());
        case 1: return long2string(pp->getHop2Sink());
        case 2: return long2string(pp->getActualHop());
        case 3: return long2string(pp->getDestinationId());
        case 4: return long2string(pp->getNextHopId());
        case 5: return long2string(pp->getSourceId());
        case 6: return long2string(pp->getSenderId());
        case 7: return long2string(pp->getMsgType());
        case 8: return long2string(pp->getMsgID());
        case 9: return long2string(pp->getMsgMark());
        case 10: return long2string(pp->getPairMark());
        case 11: return long2string(pp->getRoundMark());
        case 12: return long2string(pp->getChargeSlot());
        case 13: return long2string(pp->getEndTrans());
        case 14: return simtime2string(pp->getSendTime());
        default: return "";
    }
}

bool metaFrameDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    metaFrame *pp = (metaFrame *)object; (void)pp;
    switch (field) {
        case 0: pp->setFindCount(string2long(value)); return true;
        case 1: pp->setHop2Sink(string2long(value)); return true;
        case 2: pp->setActualHop(string2long(value)); return true;
        case 3: pp->setDestinationId(string2long(value)); return true;
        case 4: pp->setNextHopId(string2long(value)); return true;
        case 5: pp->setSourceId(string2long(value)); return true;
        case 6: pp->setSenderId(string2long(value)); return true;
        case 7: pp->setMsgType(string2long(value)); return true;
        case 8: pp->setMsgID(string2long(value)); return true;
        case 9: pp->setMsgMark(string2long(value)); return true;
        case 10: pp->setPairMark(string2long(value)); return true;
        case 11: pp->setRoundMark(string2long(value)); return true;
        case 12: pp->setChargeSlot(string2long(value)); return true;
        case 13: pp->setEndTrans(string2long(value)); return true;
        case 14: pp->setSendTime(string2simtime(value)); return true;
        default: return false;
    }
}

const char *metaFrameDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *metaFrameDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    metaFrame *pp = (metaFrame *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}


