/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_INTERFERENCE_HELPER_H
#define MMWAVE_INTERFERENCE_HELPER_H
#include <map>
#include "ns3/nstime.h"
#include "mmwave-spectrum-value-helper.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    class MmWavePpdu;
    class MmWavePsdu;
    class MmWaveErrorRateModel;

    typedef std::map <MmWaveSpectrumBand, double> RxPowerWattPerChannelBand;

    class MmWaveEvent : public SimpleRefCount<MmWaveEvent>
    {
    public:

        MmWaveEvent (Ptr<const MmWavePpdu> ppdu, MmWaveTxVector txVector, Time duration, RxPowerWattPerChannelBand rxPower);
        ~MmWaveEvent ();

        Ptr<const MmWavePpdu> GetPpdu () const;
        Time GetStartTime () const;
        Time GetEndTime () const;
        Time GetDuration () const;
        double GetRxPowerW () const;
        double GetRxPowerW (MmWaveSpectrumBand band) const;
        RxPowerWattPerChannelBand GetRxPowerWPerBand () const;
        MmWaveTxVector GetTxVector () const;

    private:
        Ptr<const MmWavePpdu> m_ppdu;           //!< PPDU
        MmWaveTxVector m_txVector;              //!< TXVECTOR
        Time m_startTime;                     //!< start time
        Time m_endTime;                       //!< end time
        RxPowerWattPerChannelBand m_rxPowerW; //!< received power in watts per band
    };

    std::ostream& operator<< (std::ostream& os, const MmWaveEvent &event);

    class MmWaveInterferenceHelper
    {
    public:

        struct SnrPer
        {
            double snr; ///< SNR in linear scale
            double per; ///< PER
        };

        MmWaveInterferenceHelper ();
        ~MmWaveInterferenceHelper ();

        void AddBand (MmWaveSpectrumBand band);
        void RemoveBands ();
        void SetNoiseFigure (double value);
        void SetErrorRateModel (const Ptr<MmWaveErrorRateModel> rate);
        Ptr<MmWaveErrorRateModel> GetErrorRateModel () const;
        void SetNumberOfReceiveAntennas (uint8_t rx);
        Time GetEnergyDuration (double energyW, MmWaveSpectrumBand band) const;
        Ptr<MmWaveEvent> Add (Ptr<const MmWavePpdu> ppdu, MmWaveTxVector txVector, Time duration, RxPowerWattPerChannelBand rxPower);
        void AddForeignSignal (Time duration, RxPowerWattPerChannelBand rxPower);
        struct MmWaveInterferenceHelper::SnrPer CalculatePayloadSnrPer (Ptr<MmWaveEvent> event, uint16_t channelWidth, MmWaveSpectrumBand band, std::pair<Time, Time> relativeMpduStartStop) const;
        double CalculateSnr (Ptr<MmWaveEvent> event, uint16_t channelWidth, uint8_t nss, MmWaveSpectrumBand band) const;
        struct MmWaveInterferenceHelper::SnrPer CalculatePhyHeaderSnrPer (Ptr<MmWaveEvent> event, MmWaveSpectrumBand band) const;
        void NotifyRxStart ();
        void NotifyRxEnd ();
        void EraseEvents ();

        double DbmToW (double dbm);
        double DbToRatio (double db);
        double WToDbm (double w);
        double RatioToDb (double ratio) const;

    protected:
        double CalculateSnr (double signal, double noiseInterference, uint16_t channelWidth, uint8_t nss) const;
        double CalculateChunkSuccessRate (double snir, Time duration, MmWaveMode mode, MmWaveTxVector txVector) const;

    private:

        class NiChange
        {
        public:
            NiChange (double power, Ptr<MmWaveEvent> event);
            double GetPower () const;
            void AddPower (double power);
            Ptr<MmWaveEvent> GetEvent () const;

        private:
            double m_power; ///< power in watts
            Ptr<MmWaveEvent> m_event; ///< event
        };

        typedef std::multimap<Time, NiChange> NiChanges;
        typedef std::map <MmWaveSpectrumBand, NiChanges> NiChangesPerBand;
        void AppendEvent (Ptr<MmWaveEvent> event);
        double CalculateNoiseInterferenceW (Ptr<MmWaveEvent> event, NiChangesPerBand *nis, MmWaveSpectrumBand band) const;
        double CalculatePayloadChunkSuccessRate (double snir, Time duration, MmWaveTxVector txVector) const;
        double CalculatePayloadPer (Ptr<const MmWaveEvent> event, uint16_t channelWidth, NiChangesPerBand *nis, MmWaveSpectrumBand band, std::pair<Time, Time> window) const;
        double CalculatePhyHeaderPer (Ptr<const MmWaveEvent> event, NiChangesPerBand *nis, MmWaveSpectrumBand band) const;

        double m_noiseFigure;                                    //!< noise figure (linear)
        Ptr<MmWaveErrorRateModel> m_errorRateModel;                    //!< error rate model
        uint8_t m_numRxAntennas;                                 //!< the number of RX antennas in the corresponding receiver
        NiChangesPerBand m_niChangesPerBand;                     //!< NI Changes for each band
        std::map <MmWaveSpectrumBand, double> m_firstPowerPerBand; //!< first power of each band in watts
        bool m_rxing;                                            //!< flag whether it is in receiving state

        NiChanges::const_iterator GetNextPosition (Time moment, MmWaveSpectrumBand band) const;
        NiChanges::const_iterator GetPreviousPosition (Time moment, MmWaveSpectrumBand band) const;
        NiChanges::iterator AddNiChangeEvent (Time moment, NiChange change, MmWaveSpectrumBand band);
    };

} //namespace ns3

#endif //MMWAVE_INTERFERENCE_HELPER_H
