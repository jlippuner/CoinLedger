#ifndef SRC_PRICES_CANDLES_HPP_
#define SRC_PRICES_CANDLES_HPP_

#include <string>
#include <vector>

class Candles {
 public:
  Candles(const std::string& instrument, const std::string& comment,
      const std::string timeframe, const std::vector<double>& times,
      const std::vector<double>& opens, const std::vector<double>& highs,
      const std::vector<double>& lows, const std::vector<double>& closes,
      const std::vector<double>& volumes,
      const std::vector<double>& volumeAveragedPrices);

  static Candles ReadFromH5(const std::string& filePath);

  std::string Instrument() const { return mInstrument; }
  std::string Comment() const { return mComment; }
  std::string Timeframe() const { return mTimeFrame; }

  const std::vector<double>& Times() const { return mTimes; }
  const std::vector<double>& Opens() const { return mOpens; }
  const std::vector<double>& Highs() const { return mHighs; }
  const std::vector<double>& Lows() const { return mLows; }
  const std::vector<double>& Closes() const { return mCloses; }
  const std::vector<double>& Volumes() const { return mVolumes; }
  const std::vector<double>& VolumeAveragedPrices() const {
    return mVolumeAveragedPrices;
  }

 private:
  std::string mInstrument;
  std::string mComment;
  std::string mTimeFrame;

  std::vector<double> mTimes;  // seconds since UNIX epoch, GMT, start of candle
  std::vector<double> mOpens;
  std::vector<double> mHighs;
  std::vector<double> mLows;
  std::vector<double> mCloses;
  std::vector<double> mVolumes;
  std::vector<double> mVolumeAveragedPrices;
};

#endif  // SRC_PRICES_CANDLES_HPP_
