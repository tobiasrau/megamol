#pragma once

template <class DATA, class CALL>
class FrameImpl {
public:
    bool FillCall(CALL& c);
    
private:
};

template <class DATA, class CALL>
class AbstractDataIO {
public:

    using Frame = FrameImpl<DATA, CALL>;

    virtual bool ReadFrame(Frame* f) {
        return static_cast<DATA*>(this)->ReadFrame(f);
    }
    virtual bool WriteFrame(Frame* f) = 0;
    virtual bool PopulateMetaData() = 0;
    virtual bool OpenFileRead() = 0;
    virtual bool OpenFileWrite() = 0;
};

template <class CALL>
class MMPLDDataIO : public AbstractDataIO<MMPLDDataIO<CALL>, CALL> {
    
};

template <class DATA, class CALL>
class AnimDataModule {
public:
private:
    AbstractDataIO<DATA, CALL>* impl;
};
