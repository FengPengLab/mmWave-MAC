/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MODE_H
#define MMWAVE_MODE_H
#include <vector>
#include <cstdint>
#include "ns3/attribute-helper.h"
#include "ns3/object.h"
#include "ns3/fatal-error.h"

namespace ns3 {

#define SU_STA_ID 65535

    class MmWaveTxVector;

    enum MmWaveModulationClass
    {
        MMWAVE_MOD_CLASS_UNKNOWN = 0,
        MMWAVE_MOD_CLASS_OFDM
    };

    enum MmWaveCodeRate
    {
        MMWAVE_CODE_RATE_UNDEFINED,
        MMWAVE_CODE_RATE_3_4,
        MMWAVE_CODE_RATE_2_3,
        MMWAVE_CODE_RATE_1_2,
        MMWAVE_CODE_RATE_5_6
    };

    class MmWaveMode
    {
    public:
        uint64_t GetPhyRate (uint16_t channelWidth, uint16_t guardInterval, uint8_t nss) const;
        uint64_t GetPhyRate (MmWaveTxVector txVector) const;
        uint64_t GetDataRate (uint16_t channelWidth, uint16_t guardInterval, uint8_t nss) const;
        uint64_t GetDataRate (MmWaveTxVector txVector) const;
        uint64_t GetDataRate (uint16_t channelWidth) const;
        MmWaveCodeRate GetCodeRate () const;
        uint16_t GetConstellationSize () const;
        uint8_t GetMcsValue () const;
        std::string GetUniqueName () const;
        bool IsMandatory () const;
        uint32_t GetUid () const;
        MmWaveModulationClass GetModulationClass () const;
        bool IsHigherCodeRate (MmWaveMode mode) const;
        bool IsHigherDataRate (MmWaveMode mode) const;
        MmWaveMode ();
        MmWaveMode (std::string name);

    private:
        friend class MmWaveModeFactory;
        MmWaveMode (uint32_t uid);
        uint32_t m_uid; ///< UID
    };

    bool operator == (const MmWaveMode &a, const MmWaveMode &b);
    bool operator < (const MmWaveMode &a, const MmWaveMode &b);
    std::ostream & operator << (std::ostream & os, const MmWaveMode &mode);
    std::istream & operator >> (std::istream &is, MmWaveMode &mode);

    ATTRIBUTE_HELPER_HEADER (MmWaveMode);

    typedef std::vector<MmWaveMode> MmWaveModeList;
    typedef MmWaveModeList::const_iterator MmWaveModeListIterator;

    class MmWaveModeFactory
    {
    public:
        static MmWaveMode CreateMmWaveMcs (std::string uniqueName, uint8_t mcsValue, MmWaveModulationClass modClass);
        static MmWaveMode CreateMmWaveMode (std::string uniqueName, MmWaveModulationClass modClass, bool isMandatory, MmWaveCodeRate codingRate, uint16_t constellationSize);
    private:
        friend class MmWaveMode;
        friend std::istream & operator >> (std::istream &is, MmWaveMode &mode);

        static MmWaveModeFactory* GetFactory ();
        MmWaveModeFactory ();
        struct MmWaveModeItem
        {
            std::string uniqueUid; ///< unique UID
            MmWaveModulationClass modClass; ///< modulation class
            uint16_t constellationSize; ///< constellation size
            MmWaveCodeRate codingRate; ///< coding rate
            bool isMandatory; ///< flag to indicate whether this mode is mandatory
            uint8_t mcsValue; ///< MCS value
        };
        MmWaveMode Search (std::string name) const;
        uint32_t AllocateUid (std::string uniqueUid);
        MmWaveModeItem* Get (uint32_t uid);

        typedef std::vector<MmWaveModeItem> MmWaveModeItemList;
        MmWaveModeItemList m_itemList; ///< item list
    };

} //namespace ns3
#endif //SRC_MMWAVE_MODE_H
