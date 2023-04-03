/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <map>
#include <cmath>
#include "ns3/log.h"
#include "ns3/fatal-error.h"
#include "ns3/assert.h"
#include "mmwave-spectrum-value-helper.h"
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveSpectrumValueHelper");

    struct MmWaveSpectrumModelId
    {
        MmWaveSpectrumModelId (uint32_t f, uint16_t w, double b, uint16_t g);
        uint32_t m_centerFrequency; ///< center frequency (in MHz)
        uint16_t m_channelWidth;     ///< channel width (in MHz)
        double m_bandBandwidth;     ///< width of each band (in Hz)
        uint16_t m_guardBandwidth;   ///< guard band width (in MHz)
    };

    MmWaveSpectrumModelId::MmWaveSpectrumModelId (uint32_t f, uint16_t w, double b, uint16_t g)
            : m_centerFrequency (f),
              m_channelWidth (w),
              m_bandBandwidth (b),
              m_guardBandwidth (g)
    {
        NS_LOG_FUNCTION (this << f << w << b << g);
    }

    MmWaveSpectrumValueHelper::~MmWaveSpectrumValueHelper ()
    {
    }

    bool
    operator < (const MmWaveSpectrumModelId& a, const MmWaveSpectrumModelId& b)
    {
        return ( (a.m_centerFrequency < b.m_centerFrequency)
                 || ((a.m_centerFrequency == b.m_centerFrequency)
                     && (a.m_channelWidth < b.m_channelWidth))
                 || ((a.m_centerFrequency == b.m_centerFrequency)
                     && (a.m_channelWidth == b.m_channelWidth)
                     && (a.m_bandBandwidth < b.m_bandBandwidth))
                 || ((a.m_centerFrequency == b.m_centerFrequency)
                     && (a.m_channelWidth == b.m_channelWidth)
                     && (a.m_bandBandwidth == b.m_bandBandwidth)
                     && (a.m_guardBandwidth < b.m_guardBandwidth)));
    }

    static std::map<MmWaveSpectrumModelId, Ptr<SpectrumModel> > g_mmWaveSpectrumModelMap; ///< static initializer for the class

    Ptr<SpectrumModel>
    MmWaveSpectrumValueHelper::GetSpectrumModel (uint32_t centerFrequency, uint16_t channelWidth, uint32_t bandBandwidth, uint16_t guardBandwidth)
    {
        NS_LOG_FUNCTION (centerFrequency << channelWidth << bandBandwidth << guardBandwidth);
        Ptr<SpectrumModel> ret;
        MmWaveSpectrumModelId key (centerFrequency, channelWidth, bandBandwidth, guardBandwidth);
        std::map<MmWaveSpectrumModelId, Ptr<SpectrumModel> >::iterator it = g_mmWaveSpectrumModelMap.find (key);
        if (it != g_mmWaveSpectrumModelMap.end ())
        {
            ret = it->second;
        }
        else
        {
            Bands bands;
            double centerFrequencyHz = centerFrequency * 1e6;
            double bandwidth = (channelWidth + (2.0 * guardBandwidth)) * 1e6;
            uint32_t numBands = static_cast<uint32_t> ((bandwidth / bandBandwidth) + 0.5);
            NS_ASSERT (numBands > 0);
            if (numBands % 2 == 0)
            {
                numBands += 1;
            }
            NS_ASSERT_MSG (numBands % 2 == 1, "Number of bands should be odd");
            NS_LOG_DEBUG ("Num bands " << numBands << " band bandwidth " << bandBandwidth);

            double startingFrequencyHz = centerFrequencyHz - (numBands / 2 * bandBandwidth) - bandBandwidth / 2;
            for (size_t i = 0; i < numBands; i++)
            {
                BandInfo info;
                double f = startingFrequencyHz + (i * bandBandwidth);
                info.fl = f;
                f += bandBandwidth / 2;
                info.fc = f;
                f += bandBandwidth / 2;
                info.fh = f;
                NS_LOG_DEBUG ("creating band " << i << " (" << info.fl << ":" << info.fc << ":" << info.fh << ")");
                bands.push_back (info);
            }
            ret = Create<SpectrumModel> (bands);
            g_mmWaveSpectrumModelMap.insert (std::pair<MmWaveSpectrumModelId, Ptr<SpectrumModel> > (key, ret));
        }
        NS_LOG_LOGIC ("returning SpectrumModel::GetUid () == " << ret->GetUid ());
        return ret;
    }

    Ptr<SpectrumValue>
    MmWaveSpectrumValueHelper::CreateMmWaveOfdmTxPowerSpectralDensity (uint32_t centerFrequency,
                                                                       uint16_t channelWidth,
                                                                       double txPowerW,
                                                                       uint16_t guardBandwidth,
                                                                       double minInnerBandDbr,
                                                                       double minOuterBandDbr,
                                                                       double lowestPointDbr)
    {
        NS_LOG_FUNCTION (centerFrequency << channelWidth << txPowerW << guardBandwidth << minInnerBandDbr << minOuterBandDbr << lowestPointDbr);
        uint32_t bandBandwidth = 78125;
        Ptr<SpectrumValue> c = Create<SpectrumValue> (GetSpectrumModel (centerFrequency, channelWidth, bandBandwidth, guardBandwidth));
        uint32_t nGuardBands = static_cast<uint32_t> (((2 * guardBandwidth * 1e6) / bandBandwidth) + 0.5);
        uint32_t nAllocatedBands = static_cast<uint32_t> (((channelWidth * 1e6) / bandBandwidth) + 0.5);
        NS_ASSERT_MSG (c->GetSpectrumModel ()->GetNumBands () == (nAllocatedBands + nGuardBands + 1), "Unexpected number of bands " << c->GetSpectrumModel ()->GetNumBands ());
        double txPowerPerBandW = 0.0;
        uint32_t start1;
        uint32_t stop1;
        uint32_t start2;
        uint32_t stop2;
        uint32_t start3;
        uint32_t stop3;
        uint32_t start4;
        uint32_t stop4;
        uint32_t start5;
        uint32_t stop5;
        uint32_t start6;
        uint32_t stop6;
        uint32_t start7;
        uint32_t stop7;
        uint32_t start8;
        uint32_t stop8;
        uint32_t start9;
        uint32_t stop9;
        uint32_t start10;
        uint32_t stop10;
        uint32_t start11;
        uint32_t stop11;
        uint32_t start12;
        uint32_t stop12;
        uint32_t start13;
        uint32_t stop13;
        uint32_t start14;
        uint32_t stop14;
        uint32_t start15;
        uint32_t stop15;
        uint32_t start16;
        uint32_t stop16;
        uint32_t start17;
        uint32_t stop17;
        uint32_t start18;
        uint32_t stop18;
        uint32_t start19;
        uint32_t stop19;
        uint32_t start20;
        uint32_t stop20;
        uint32_t start21;
        uint32_t stop21;
        uint32_t start22;
        uint32_t stop22;
        uint32_t start23;
        uint32_t stop23;
        uint32_t start24;
        uint32_t stop24;
        uint32_t start25;
        uint32_t stop25;
        uint32_t start26;
        uint32_t stop26;
        uint32_t start27;
        uint32_t stop27;
        uint32_t start28;
        uint32_t stop28;
        uint32_t start29;
        uint32_t stop29;
        uint32_t start30;
        uint32_t stop30;
        uint32_t start31;
        uint32_t stop31;
        uint32_t start32;
        uint32_t stop32;

        //Prepare spectrum mask specific variables
        uint32_t innerSlopeWidth = static_cast<uint32_t> ((1e6 / bandBandwidth) + 0.5); //size in number of subcarriers of the inner band
        std::vector <MmWaveSpectrumBand> subBands; //list of data/pilot-containing subBands (sent at 0dBr)
        MmWaveSpectrumBand maskBand (0, nAllocatedBands + nGuardBands);
        switch (channelWidth)
        {
            case 80:
                // 996 subcarriers (980 data + 16 pilot)
                txPowerPerBandW = txPowerW / 996;
                // skip the guard band and 12 subbands, then place power in 498 subbands, then
                // skip 5 DC, then place power in 498 subbands, then skip
                // the final 11 subbands and the guard band.
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1;
                start2 = stop1 + 6;
                stop2 = start2 + 498 - 1;
                subBands.push_back (std::make_pair (start1, stop1));
                subBands.push_back (std::make_pair (start2, stop2));
                break;
            case 160:
                // 2 x 996 subcarriers (2 x 80 MHZ bands)
                txPowerPerBandW = txPowerW / (2 * 996);
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1;
                start2 = stop1 + 6;
                stop2 = start2 + 498 - 1;
                start3 = stop2 + (2 * 12);
                stop3 = start3 + 498 - 1;
                start4 = stop3 + 6;
                stop4 = start4 + 498 - 1;
                subBands.push_back (std::make_pair (start1, stop1));
                subBands.push_back (std::make_pair (start2, stop2));
                subBands.push_back (std::make_pair (start3, stop3));
                subBands.push_back (std::make_pair (start4, stop4));
                break;
            case 320:
                // 4 x 996 subcarriers (4 x 80 MHZ bands)
                txPowerPerBandW = txPowerW / (4 * 996);
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1;
                start2 = stop1 + 6;
                stop2 = start2 + 498 - 1;
                start3 = stop2 + (2 * 12);
                stop3 = start3 + 498 - 1;
                start4 = stop3 + 6;
                stop4 = start4 + 498 - 1;

                start5 = stop4 + (2 * 12);
                stop5 = start5 + 498 - 1;
                start6 = stop5 + 6;
                stop6 = start6 + 498 - 1;
                start7 = stop6 + (2 * 12);
                stop7 = start7 + 498 - 1;
                start8 = stop7 + 6;
                stop8 = start8 + 498 - 1;

                subBands.push_back (std::make_pair (start1, stop1));
                subBands.push_back (std::make_pair (start2, stop2));
                subBands.push_back (std::make_pair (start3, stop3));
                subBands.push_back (std::make_pair (start4, stop4));
                subBands.push_back (std::make_pair (start5, stop5));
                subBands.push_back (std::make_pair (start6, stop6));
                subBands.push_back (std::make_pair (start7, stop7));
                subBands.push_back (std::make_pair (start8, stop8));
                break;
            case 640:
                // 8 x 996 subcarriers (8 x 80 MHZ bands)
                txPowerPerBandW = txPowerW / (8 * 996);
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1;
                start2 = stop1 + 6;
                stop2 = start2 + 498 - 1;
                start3 = stop2 + (2 * 12);
                stop3 = start3 + 498 - 1;
                start4 = stop3 + 6;
                stop4 = start4 + 498 - 1;

                start5 = stop4 + (2 * 12);
                stop5 = start5 + 498 - 1;
                start6 = stop5 + 6;
                stop6 = start6 + 498 - 1;
                start7 = stop6 + (2 * 12);
                stop7 = start7 + 498 - 1;
                start8 = stop7 + 6;
                stop8 = start8 + 498 - 1;

                start9 = stop8 + (2 * 12);
                stop9 = start9 + 498 - 1;
                start10 = stop9 + 6;
                stop10 = start10 + 498 - 1;
                start11 = stop10 + (2 * 12);
                stop11 = start11 + 498 - 1;
                start12 = stop11 + 6;
                stop12 = start12 + 498 - 1;

                start13 = stop12 + (2 * 12);
                stop13 = start13 + 498 - 1;
                start14 = stop13 + 6;
                stop14 = start14 + 498 - 1;
                start15 = stop14 + (2 * 12);
                stop15 = start15 + 498 - 1;
                start16 = stop15 + 6;
                stop16 = start16 + 498 - 1;

                subBands.push_back (std::make_pair (start1, stop1));
                subBands.push_back (std::make_pair (start2, stop2));
                subBands.push_back (std::make_pair (start3, stop3));
                subBands.push_back (std::make_pair (start4, stop4));
                subBands.push_back (std::make_pair (start5, stop5));
                subBands.push_back (std::make_pair (start6, stop6));
                subBands.push_back (std::make_pair (start7, stop7));
                subBands.push_back (std::make_pair (start8, stop8));
                subBands.push_back (std::make_pair (start9, stop9));
                subBands.push_back (std::make_pair (start10, stop10));
                subBands.push_back (std::make_pair (start11, stop11));
                subBands.push_back (std::make_pair (start12, stop12));
                subBands.push_back (std::make_pair (start13, stop13));
                subBands.push_back (std::make_pair (start14, stop14));
                subBands.push_back (std::make_pair (start15, stop15));
                subBands.push_back (std::make_pair (start16, stop16));
                break;
            case 1280:
                // 16 x 996 subcarriers (16 x 80 MHZ bands)
                txPowerPerBandW = txPowerW / (16 * 996);
                start1 = (nGuardBands / 2) + 12;
                stop1 = start1 + 498 - 1;
                start2 = stop1 + 6;
                stop2 = start2 + 498 - 1;
                start3 = stop2 + (2 * 12);
                stop3 = start3 + 498 - 1;
                start4 = stop3 + 6;
                stop4 = start4 + 498 - 1;

                start5 = stop4 + (2 * 12);
                stop5 = start5 + 498 - 1;
                start6 = stop5 + 6;
                stop6 = start6 + 498 - 1;
                start7 = stop6 + (2 * 12);
                stop7 = start7 + 498 - 1;
                start8 = stop7 + 6;
                stop8 = start8 + 498 - 1;

                start9 = stop8 + (2 * 12);
                stop9 = start9 + 498 - 1;
                start10 = stop9 + 6;
                stop10 = start10 + 498 - 1;
                start11 = stop10 + (2 * 12);
                stop11 = start11 + 498 - 1;
                start12 = stop11 + 6;
                stop12 = start12 + 498 - 1;

                start13 = stop12 + (2 * 12);
                stop13 = start13 + 498 - 1;
                start14 = stop13 + 6;
                stop14 = start14 + 498 - 1;
                start15 = stop14 + (2 * 12);
                stop15 = start15 + 498 - 1;
                start16 = stop15 + 6;
                stop16 = start16 + 498 - 1;

                start17 = stop16 + (2 * 12);
                stop17 = start17 + 498 - 1;
                start18 = stop17 + 6;
                stop18 = start18 + 498 - 1;
                start19 = stop18 + (2 * 12);
                stop19 = start19 + 498 - 1;
                start20 = stop19 + 6;
                stop20 = start20 + 498 - 1;

                start21 = stop20 + (2 * 12);
                stop21 = start21 + 498 - 1;
                start22 = stop21 + 6;
                stop22 = start22 + 498 - 1;
                start23 = stop22 + (2 * 12);
                stop23 = start23 + 498 - 1;
                start24 = stop23 + 6;
                stop24 = start24 + 498 - 1;

                start25 = stop24 + (2 * 12);
                stop25 = start25 + 498 - 1;
                start26 = stop25 + 6;
                stop26 = start26 + 498 - 1;
                start27 = stop26 + (2 * 12);
                stop27 = start27 + 498 - 1;
                start28 = stop27 + 6;
                stop28 = start28 + 498 - 1;

                start29 = stop28 + (2 * 12);
                stop29 = start29 + 498 - 1;
                start30 = stop29 + 6;
                stop30 = start30 + 498 - 1;
                start31 = stop30 + (2 * 12);
                stop31 = start31 + 498 - 1;
                start32 = stop31 + 6;
                stop32 = start32 + 498 - 1;

                subBands.push_back (std::make_pair (start1, stop1));
                subBands.push_back (std::make_pair (start2, stop2));
                subBands.push_back (std::make_pair (start3, stop3));
                subBands.push_back (std::make_pair (start4, stop4));
                subBands.push_back (std::make_pair (start5, stop5));
                subBands.push_back (std::make_pair (start6, stop6));
                subBands.push_back (std::make_pair (start7, stop7));
                subBands.push_back (std::make_pair (start8, stop8));
                subBands.push_back (std::make_pair (start9, stop9));
                subBands.push_back (std::make_pair (start10, stop10));
                subBands.push_back (std::make_pair (start11, stop11));
                subBands.push_back (std::make_pair (start12, stop12));
                subBands.push_back (std::make_pair (start13, stop13));
                subBands.push_back (std::make_pair (start14, stop14));
                subBands.push_back (std::make_pair (start15, stop15));
                subBands.push_back (std::make_pair (start16, stop16));
                subBands.push_back (std::make_pair (start17, stop17));
                subBands.push_back (std::make_pair (start18, stop18));
                subBands.push_back (std::make_pair (start19, stop19));
                subBands.push_back (std::make_pair (start20, stop20));
                subBands.push_back (std::make_pair (start21, stop21));
                subBands.push_back (std::make_pair (start22, stop22));
                subBands.push_back (std::make_pair (start23, stop23));
                subBands.push_back (std::make_pair (start24, stop24));
                subBands.push_back (std::make_pair (start25, stop25));
                subBands.push_back (std::make_pair (start26, stop26));
                subBands.push_back (std::make_pair (start27, stop27));
                subBands.push_back (std::make_pair (start28, stop28));
                subBands.push_back (std::make_pair (start29, stop29));
                subBands.push_back (std::make_pair (start30, stop30));
                subBands.push_back (std::make_pair (start31, stop31));
                subBands.push_back (std::make_pair (start32, stop32));

                break;
            default:
                NS_FATAL_ERROR ("ChannelWidth " << channelWidth << " unsupported");
                break;
        }

        //Build transmit spectrum mask
        CreateSpectrumMaskForOfdm (c, subBands, maskBand,
                                   txPowerPerBandW, nGuardBands,
                                   innerSlopeWidth, minInnerBandDbr,
                                   minOuterBandDbr, lowestPointDbr);
        NormalizeSpectrumMask (c, txPowerW);
        NS_ASSERT_MSG (std::abs (txPowerW - Integral (*c)) < 1e-6, "Power allocation failed");
        return c;
    }

    Ptr<SpectrumValue>
    MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (uint32_t centerFrequency, uint16_t channelWidth, uint32_t bandBandwidth, double noiseFigure, uint16_t guardBandwidth)
    {
        Ptr<SpectrumModel> model = GetSpectrumModel (centerFrequency, channelWidth, bandBandwidth, guardBandwidth);
        return CreateNoisePowerSpectralDensity (noiseFigure, model);
    }

    Ptr<SpectrumValue>
    MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (double noiseFigureDb, Ptr<SpectrumModel> spectrumModel)
    {
        NS_LOG_FUNCTION (noiseFigureDb << spectrumModel);

        const double kT_dBm_Hz = -174.0;  // dBm/Hz
        double kT_W_Hz = DbmToW (kT_dBm_Hz);
        double noiseFigureLinear = std::pow (10.0, noiseFigureDb / 10.0);
        double noisePowerSpectralDensity =  kT_W_Hz * noiseFigureLinear;

        Ptr<SpectrumValue> noisePsd = Create <SpectrumValue> (spectrumModel);
        (*noisePsd) = noisePowerSpectralDensity;
        NS_LOG_INFO ("NoisePowerSpectralDensity has integrated power of " << Integral (*noisePsd));
        return noisePsd;
    }

    Ptr<SpectrumValue>
    MmWaveSpectrumValueHelper::CreateRfFilter (uint32_t centerFrequency, uint16_t totalChannelWidth, uint32_t bandBandwidth, uint16_t guardBandwidth, MmWaveSpectrumBand band)
    {
        uint32_t startIndex = band.first;
        uint32_t stopIndex = band.second;
        NS_LOG_FUNCTION (centerFrequency << totalChannelWidth << bandBandwidth << guardBandwidth << startIndex << stopIndex);
        Ptr<SpectrumValue> c = Create <SpectrumValue> (GetSpectrumModel (centerFrequency, totalChannelWidth, bandBandwidth, guardBandwidth));
        Bands::const_iterator bit = c->ConstBandsBegin ();
        Values::iterator vit = c->ValuesBegin ();
        vit += startIndex;
        bit += startIndex;
        for (size_t i = startIndex; i <= stopIndex; i++, vit++, bit++)
        {
            *vit = 1;
        }
        NS_LOG_LOGIC ("Added subbands " << startIndex << " to " << stopIndex << " to filter");
        return c;
    }

    void
    MmWaveSpectrumValueHelper::CreateSpectrumMaskForOfdm (Ptr<SpectrumValue> c, std::vector <MmWaveSpectrumBand> allocatedSubBands, MmWaveSpectrumBand maskBand,
                                                        double txPowerPerBandW, uint32_t nGuardBands, uint32_t innerSlopeWidth,
                                                        double minInnerBandDbr, double minOuterBandDbr, double lowestPointDbr)
    {
        NS_LOG_FUNCTION (c << allocatedSubBands.front ().first << allocatedSubBands.back ().second << maskBand.first << maskBand.second <<
                           txPowerPerBandW << nGuardBands << innerSlopeWidth << minInnerBandDbr << minOuterBandDbr << lowestPointDbr);
        uint32_t numSubBands = allocatedSubBands.size ();
        uint32_t numBands = c->GetSpectrumModel ()->GetNumBands ();
        uint32_t numMaskBands = maskBand.second - maskBand.first + 1;
        NS_ASSERT (numSubBands && numBands && numMaskBands);
        NS_LOG_LOGIC ("Power per band " << txPowerPerBandW << "W");

        //Different power levels
        double txPowerRefDbm = (10.0 * std::log10 (txPowerPerBandW * 1000.0));
        double txPowerInnerBandMinDbm = txPowerRefDbm + minInnerBandDbr;
        double txPowerMiddleBandMinDbm = txPowerRefDbm + minOuterBandDbr;
        double txPowerOuterBandMinDbm = txPowerRefDbm + lowestPointDbr; //TODO also take into account dBm/MHz constraints

        uint32_t outerSlopeWidth = nGuardBands / 4;
        uint32_t middleSlopeWidth = outerSlopeWidth - (innerSlopeWidth / 2);
        MmWaveSpectrumBand outerBandLeft (maskBand.first, maskBand.first + outerSlopeWidth - 1);
        MmWaveSpectrumBand middleBandLeft (outerBandLeft.second + 1, outerBandLeft.second + middleSlopeWidth);
        MmWaveSpectrumBand innerBandLeft (allocatedSubBands.front ().first - innerSlopeWidth, allocatedSubBands.front ().first - 1); //better to place slope based on allocated subcarriers
        MmWaveSpectrumBand flatJunctionLeft (middleBandLeft.second + 1, innerBandLeft.first - 1); //in order to handle shift due to guard subcarriers
        MmWaveSpectrumBand outerBandRight (maskBand.second - outerSlopeWidth + 1, maskBand.second); //start from outer edge to be able to compute flat junction width
        MmWaveSpectrumBand middleBandRight (outerBandRight.first - middleSlopeWidth, outerBandRight.first - 1);
        MmWaveSpectrumBand innerBandRight (allocatedSubBands.back ().second + 1, allocatedSubBands.back ().second + innerSlopeWidth);
        MmWaveSpectrumBand flatJunctionRight (innerBandRight.second + 1, middleBandRight.first - 1);
        NS_LOG_DEBUG ("outerBandLeft=[" << outerBandLeft.first << ";" << outerBandLeft.second << "] " <<
                                        "middleBandLeft=[" << middleBandLeft.first << ";" << middleBandLeft.second << "] " <<
                                        "flatJunctionLeft=[" << flatJunctionLeft.first << ";" << flatJunctionLeft.second << "] " <<
                                        "innerBandLeft=[" << innerBandLeft.first << ";" << innerBandLeft.second << "] " <<
                                        "subBands=[" << allocatedSubBands.front ().first << ";" << allocatedSubBands.back ().second << "] " <<
                                        "innerBandRight=[" << innerBandRight.first << ";" << innerBandRight.second << "] " <<
                                        "flatJunctionRight=[" << flatJunctionRight.first << ";" << flatJunctionRight.second << "] " <<
                                        "middleBandRight=[" << middleBandRight.first << ";" << middleBandRight.second << "] " <<
                                        "outerBandRight=[" << outerBandRight.first << ";" << outerBandRight.second << "] ");
        NS_ASSERT (numMaskBands == ((allocatedSubBands.back ().second - allocatedSubBands.front ().first + 1)  //equivalent to allocatedBand (includes notches and DC)
                                    + 2 * (innerSlopeWidth + middleSlopeWidth + outerSlopeWidth)
                                    + (flatJunctionLeft.second - flatJunctionLeft.first + 1) //flat junctions
                                    + (flatJunctionRight.second - flatJunctionRight.first + 1)));

        //Different slopes
        double innerSlope = (-1 * minInnerBandDbr) / innerSlopeWidth;
        double middleSlope = (-1 * (minOuterBandDbr - minInnerBandDbr)) / middleSlopeWidth;
        double outerSlope = (txPowerMiddleBandMinDbm - txPowerOuterBandMinDbm) / outerSlopeWidth;

        //Build spectrum mask
        Values::iterator vit = c->ValuesBegin ();
        Bands::const_iterator bit = c->ConstBandsBegin ();
        double txPowerW = 0.0;
        for (size_t i = 0; i < numBands; i++, vit++, bit++)
        {
            if (i < maskBand.first || i > maskBand.second) //outside the spectrum mask
            {
                txPowerW = 0.0;
            }
            else if (i <= outerBandLeft.second && i >= outerBandLeft.first) //better to put greater first (less computation)
            {
                txPowerW = DbmToW (txPowerOuterBandMinDbm + ((i - outerBandLeft.first) * outerSlope));
            }
            else if (i <= middleBandLeft.second && i >= middleBandLeft.first)
            {
                txPowerW = DbmToW (txPowerMiddleBandMinDbm + ((i - middleBandLeft.first) * middleSlope));
            }
            else if (i <= flatJunctionLeft.second && i >= flatJunctionLeft.first)
            {
                txPowerW = DbmToW (txPowerInnerBandMinDbm);
            }
            else if (i <= innerBandLeft.second && i >= innerBandLeft.first)
            {
                txPowerW = DbmToW (txPowerInnerBandMinDbm + ((i - innerBandLeft.first) * innerSlope));
            }
            else if (i <= allocatedSubBands.back ().second && i >= allocatedSubBands.front ().first) //roughly in allocated band
            {
                bool insideSubBand = false;
                for (uint32_t j = 0; !insideSubBand && j < numSubBands; j++) //continue until inside a sub-band
                {
                    insideSubBand = (i <= allocatedSubBands[j].second) && (i >= allocatedSubBands[j].first);
                }
                if (insideSubBand)
                {
                    txPowerW = txPowerPerBandW;
                }
                else
                {
                    txPowerW = DbmToW (txPowerInnerBandMinDbm);
                }
            }
            else if (i <= innerBandRight.second && i >= innerBandRight.first)
            {
                txPowerW = DbmToW (txPowerRefDbm - ((i - innerBandRight.first + 1) * innerSlope)); // +1 so as to be symmetric with left slope
            }
            else if (i <= flatJunctionRight.second && i >= flatJunctionRight.first)
            {
                txPowerW = DbmToW (txPowerInnerBandMinDbm);
            }
            else if (i <= middleBandRight.second && i >= middleBandRight.first)
            {
                txPowerW = DbmToW (txPowerInnerBandMinDbm - ((i - middleBandRight.first + 1) * middleSlope)); // +1 so as to be symmetric with left slope
            }
            else if (i <= outerBandRight.second && i >= outerBandRight.first)
            {
                txPowerW = DbmToW (txPowerMiddleBandMinDbm - ((i - outerBandRight.first + 1) * outerSlope)); // +1 so as to be symmetric with left slope
            }
            else
            {
                NS_FATAL_ERROR ("Should have handled all cases");
            }
            double txPowerDbr = 10 * std::log10 (txPowerW / txPowerPerBandW);
            NS_LOG_LOGIC (uint32_t (i) << " -> " << txPowerDbr);
            *vit = txPowerW / (bit->fh - bit->fl);
        }
        NS_LOG_INFO ("Added signal power to subbands " << allocatedSubBands.front ().first << "-" << allocatedSubBands.back ().second);
    }

    void
    MmWaveSpectrumValueHelper::NormalizeSpectrumMask (Ptr<SpectrumValue> c, double txPowerW)
    {
        NS_LOG_FUNCTION (c << txPowerW);
        //Normalize power so that total signal power equals transmit power
        double currentTxPowerW = Integral (*c);
        double normalizationRatio = currentTxPowerW / txPowerW;
        NS_LOG_LOGIC ("Current power: " << currentTxPowerW << "W vs expected power: " << txPowerW << "W" << " -> ratio (C/E) = " << normalizationRatio);
        Values::iterator vit = c->ValuesBegin ();
        for (size_t i = 0; i < c->GetSpectrumModel ()->GetNumBands (); i++, vit++)
        {
            *vit = (*vit) / normalizationRatio;
        }
    }

    double
    MmWaveSpectrumValueHelper::DbmToW (double dBm)
    {
        return std::pow (10.0, 0.1 * (dBm - 30.0));
    }

} // namespace ns3