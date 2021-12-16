/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <cmath>
#include "ns3/log.h"
#include "mmwave-mode.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    bool operator == (const MmWaveMode &a, const MmWaveMode &b)
    {
        return a.GetUid () == b.GetUid ();
    }

    bool operator < (const MmWaveMode &a, const MmWaveMode &b)
    {
        return a.GetUid () < b.GetUid ();
    }

    std::ostream & operator << (std::ostream & os, const MmWaveMode &mode)
    {
        os << mode.GetUniqueName ();
        return os;
    }

    std::istream & operator >> (std::istream &is, MmWaveMode &mode)
    {
        std::string str;
        is >> str;
        mode = MmWaveModeFactory::GetFactory ()->Search (str);
        return is;
    }

    uint64_t
    MmWaveMode::GetPhyRate (uint16_t channelWidth, uint16_t guardInterval, uint8_t nss) const
    {
        //TODO: nss > 4 not supported yet
        NS_ASSERT (nss <= 4);
        uint64_t dataRate, phyRate;
        dataRate = GetDataRate (channelWidth, guardInterval, nss);
        switch (GetCodeRate ())
        {
            case MMWAVE_CODE_RATE_5_6:
                phyRate = dataRate * 6 / 5;
                break;
            case MMWAVE_CODE_RATE_3_4:
                phyRate = dataRate * 4 / 3;
                break;
            case MMWAVE_CODE_RATE_2_3:
                phyRate = dataRate * 3 / 2;
                break;
            case MMWAVE_CODE_RATE_1_2:
                phyRate = dataRate * 2 / 1;
                break;
            case MMWAVE_CODE_RATE_UNDEFINED:
            default:
                phyRate = dataRate;
                break;
        }
        return phyRate;
    }

    uint64_t
    MmWaveMode::GetPhyRate (MmWaveTxVector txVector) const
    {
        return GetPhyRate (txVector.GetChannelWidth (), txVector.GetGuardInterval (), txVector.GetNss ());
    }

    uint64_t
    MmWaveMode::GetDataRate (uint16_t channelWidth) const
    {
        return GetDataRate (channelWidth, 3200, 1);
    }

    uint64_t
    MmWaveMode::GetDataRate (MmWaveTxVector txVector) const
    {
        uint16_t bw = txVector.GetChannelWidth ();
        uint8_t nss = txVector.GetNss ();
        return GetDataRate (bw, txVector.GetGuardInterval (), nss);
    }

    uint64_t
    MmWaveMode::GetDataRate (uint16_t channelWidth, uint16_t guardInterval, uint8_t nss) const
    {
        //TODO: nss > 4 not supported yet
        NS_ASSERT (nss <= 4);
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        uint64_t dataRate = 0;
        uint16_t usableSubCarriers = 0;
        double symbolRate = 0;
        double codingRate = 0;
        uint16_t numberOfBitsPerSubcarrier = static_cast<uint16_t> (log2 (GetConstellationSize ()));

        if (item->modClass == MMWAVE_MOD_CLASS_OFDM)
        {
            NS_ASSERT (guardInterval == 800 || guardInterval == 1600 || guardInterval == 3200);
            symbolRate = (1 / (12.8 + (static_cast<double> (guardInterval) / 1000))) * 1e6;

            switch (channelWidth)
            {
                default:
                case 320:
                    usableSubCarriers = 3920;
                    break;
                case 640:
                    usableSubCarriers = 7840;
                    break;
                case 1280:
                    usableSubCarriers = 15680;
                    break;
            }

            switch (GetCodeRate ())
            {
                case MMWAVE_CODE_RATE_5_6:
                    codingRate = (5.0 / 6.0);
                    break;
                case MMWAVE_CODE_RATE_3_4:
                    codingRate = (3.0 / 4.0);
                    break;
                case MMWAVE_CODE_RATE_2_3:
                    codingRate = (2.0 / 3.0);
                    break;
                case MMWAVE_CODE_RATE_1_2:
                    codingRate = (1.0 / 2.0);
                    break;
                case MMWAVE_CODE_RATE_UNDEFINED:
                default:
                    NS_FATAL_ERROR ("trying to get datarate for a mcs without any coding rate defined with nss: " << +nss);
                    break;
            }

            dataRate = lrint (ceil (symbolRate * usableSubCarriers * numberOfBitsPerSubcarrier * codingRate));
        }
        else
        {
            NS_ASSERT ("undefined datarate for the modulation class!");
        }
        dataRate *= nss; // number of spatial streams
        return dataRate;
    }

    MmWaveCodeRate
    MmWaveMode::GetCodeRate () const
    {
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        if (item->modClass == MMWAVE_MOD_CLASS_OFDM)
        {
            switch (item->mcsValue)
            {
                case 0:
                    return MMWAVE_CODE_RATE_1_2;
                case 1:
                    return MMWAVE_CODE_RATE_2_3;
                case 2:
                    return MMWAVE_CODE_RATE_3_4;
                case 3:
                    return MMWAVE_CODE_RATE_5_6;
                default:
                    return MMWAVE_CODE_RATE_UNDEFINED;
            }
        }
        else
        {
            return item->codingRate;
        }
    }

    uint16_t
    MmWaveMode::GetConstellationSize () const
    {
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        if (item->modClass == MMWAVE_MOD_CLASS_OFDM)
        {
            switch (item->mcsValue)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                default:
                    return 254;
            }
        }
        else
        {
            return item->constellationSize;
        }
    }

    std::string
    MmWaveMode::GetUniqueName () const
    {
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        return item->uniqueUid;
    }

    bool
    MmWaveMode::IsMandatory () const
    {
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        return item->isMandatory;
    }

    uint8_t
    MmWaveMode::GetMcsValue () const
    {
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        if (item->modClass == MMWAVE_MOD_CLASS_OFDM)
        {
            return item->mcsValue;
        }
        else
        {
            NS_ASSERT (false);
            return 0;
        }
    }

    uint32_t
    MmWaveMode::GetUid () const
    {
        return m_uid;
    }

    MmWaveModulationClass
    MmWaveMode::GetModulationClass () const
    {
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        return item->modClass;
    }

    bool
    MmWaveMode::IsHigherCodeRate (MmWaveMode mode) const
    {
        MmWaveCodeRate codeRate = mode.GetCodeRate ();
        switch (GetCodeRate ())
        {
            case MMWAVE_CODE_RATE_1_2:
                return false; //This is the smallest code rate.
            case MMWAVE_CODE_RATE_2_3:
                return (codeRate == MMWAVE_CODE_RATE_1_2);
            case MMWAVE_CODE_RATE_3_4:
                return (codeRate == MMWAVE_CODE_RATE_1_2 || codeRate == MMWAVE_CODE_RATE_2_3);
            case MMWAVE_CODE_RATE_5_6:
                return (codeRate == MMWAVE_CODE_RATE_1_2 || codeRate == MMWAVE_CODE_RATE_2_3 || codeRate == MMWAVE_CODE_RATE_3_4);
            default:
                NS_FATAL_ERROR ("MmWave Code Rate not defined");
                return false;
        }
    }

    bool
    MmWaveMode::IsHigherDataRate (MmWaveMode mode) const
    {
        MmWaveModeFactory::MmWaveModeItem *item = MmWaveModeFactory::GetFactory ()->Get (m_uid);
        switch (item->modClass)
        {
            case MMWAVE_MOD_CLASS_OFDM:
                if (GetConstellationSize () > mode.GetConstellationSize ())
                {
                    return true;
                }
                else if (GetConstellationSize () == mode.GetConstellationSize ())
                {
                    return IsHigherCodeRate (mode);
                }
                else
                {
                    return false;
                }
            default:
                NS_FATAL_ERROR ("Modulation class not defined");
                return false;
        }
    }

    MmWaveMode::MmWaveMode ()
            : m_uid (0)
    {
    }

    MmWaveMode::MmWaveMode (uint32_t uid)
            : m_uid (uid)
    {
    }

    MmWaveMode::MmWaveMode (std::string name)
    {
        *this = MmWaveModeFactory::GetFactory ()->Search (name);
    }

    ATTRIBUTE_HELPER_CPP (MmWaveMode);

    MmWaveModeFactory::MmWaveModeFactory ()
    {
    }

    MmWaveMode
    MmWaveModeFactory::CreateMmWaveMcs (std::string uniqueName, uint8_t mcsValue, MmWaveModulationClass modClass)
    {
        MmWaveModeFactory *factory = GetFactory ();
        uint32_t uid = factory->AllocateUid (uniqueName);
        MmWaveModeItem *item = factory->Get (uid);
        item->uniqueUid = uniqueName;
        item->modClass = modClass;
        NS_ASSERT (modClass == MMWAVE_MOD_CLASS_OFDM);
        item->mcsValue = mcsValue;
        item->constellationSize = 0;
        item->codingRate = MMWAVE_CODE_RATE_UNDEFINED;
        item->isMandatory = false;

        return MmWaveMode (uid);
    }

    MmWaveMode
    MmWaveModeFactory::CreateMmWaveMode (std::string uniqueName, MmWaveModulationClass modClass, bool isMandatory, MmWaveCodeRate codingRate, uint16_t constellationSize)
    {
        MmWaveModeFactory *factory = GetFactory ();
        uint32_t uid = factory->AllocateUid (uniqueName);
        MmWaveModeItem *item = factory->Get (uid);
        item->uniqueUid = uniqueName;
        item->modClass = modClass;
        NS_ASSERT (modClass != MMWAVE_MOD_CLASS_UNKNOWN);
        item->codingRate = codingRate;

        if (codingRate == MMWAVE_CODE_RATE_UNDEFINED)
        {
            NS_FATAL_ERROR ("Error in creation of MmWaveMode named " << uniqueName << std::endl);
        }

        item->constellationSize = constellationSize;
        item->isMandatory = isMandatory;

        NS_ASSERT (modClass == MMWAVE_MOD_CLASS_OFDM);
        item->mcsValue = 0;

        return MmWaveMode (uid);
    }

    MmWaveMode
    MmWaveModeFactory::Search (std::string name) const
    {
        MmWaveModeItemList::const_iterator i;
        uint32_t j = 0;
        for (i = m_itemList.begin (); i != m_itemList.end (); i++)
        {
            if (i->uniqueUid == name)
            {
                return MmWaveMode (j);
            }
            j++;
        }

        NS_LOG_UNCOND ("Could not find match for MmWaveMode named \""
                               << name << "\". Valid options are:");
        for (i = m_itemList.begin (); i != m_itemList.end (); i++)
        {
            NS_LOG_UNCOND ("  " << i->uniqueUid);
        }
        NS_FATAL_ERROR ("");
        return MmWaveMode (0);
    }

    uint32_t
    MmWaveModeFactory::AllocateUid (std::string uniqueUid)
    {
        uint32_t j = 0;
        for (MmWaveModeItemList::const_iterator i = m_itemList.begin ();
             i != m_itemList.end (); i++)
        {
            if (i->uniqueUid == uniqueUid)
            {
                return j;
            }
            j++;
        }
        uint32_t uid = static_cast<uint32_t> (m_itemList.size ());
        m_itemList.push_back (MmWaveModeItem ());
        return uid;
    }

    MmWaveModeFactory::MmWaveModeItem *
    MmWaveModeFactory::Get (uint32_t uid)
    {
        NS_ASSERT (uid < m_itemList.size ());
        return &m_itemList[uid];
    }

    MmWaveModeFactory *
    MmWaveModeFactory::GetFactory ()
    {
        static bool isFirstTime = true;
        static MmWaveModeFactory factory;
        if (isFirstTime)
        {
            uint32_t uid = factory.AllocateUid ("Invalid-MmWaveMode");
            MmWaveModeItem *item = factory.Get (uid);
            item->uniqueUid = "Invalid-MmWaveMode";
            item->modClass = MMWAVE_MOD_CLASS_UNKNOWN;
            item->constellationSize = 0;
            item->codingRate = MMWAVE_CODE_RATE_UNDEFINED;
            item->isMandatory = false;
            item->mcsValue = 0;
            isFirstTime = false;
        }
        return &factory;
    }

} //namespace ns3