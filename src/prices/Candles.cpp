#include "Candles.hpp"

#include <cmath>

#include "H5Helper.hpp"

Candles::Candles(const std::string& instrument, const std::string& comment,
    const std::string timeframe, const std::vector<double>& times,
    const std::vector<double>& opens, const std::vector<double>& highs,
    const std::vector<double>& lows, const std::vector<double>& closes,
    const std::vector<double>& volumes,
    const std::vector<double>& volumeAveragedPrices)
    : mInstrument(instrument),
      mComment(comment),
      mTimeFrame(timeframe),
      mTimes(times),
      mOpens(opens),
      mHighs(highs),
      mLows(lows),
      mCloses(closes),
      mVolumes(volumes),
      mVolumeAveragedPrices(volumeAveragedPrices) {}

Candles Candles::ReadFromH5(const std::string& filePath) {
  H5::H5File file(filePath, H5F_ACC_RDONLY);

  auto instrument = GetStringAttribute(file, "instrument");
  auto comment = GetStringAttribute(file, "comment");
  auto timeframe = GetStringAttribute(file, "timeframe");

  auto times = ReadDataFromH5File<double>(file, "times");
  auto opens = ReadDataFromH5File<double>(file, "opens");
  auto highs = ReadDataFromH5File<double>(file, "highs");
  auto lows = ReadDataFromH5File<double>(file, "lows");
  auto closes = ReadDataFromH5File<double>(file, "closes");
  auto volumes = ReadDataFromH5File<double>(file, "volumes");
  auto volumeAveragedPrices =
      ReadDataFromH5File<double>(file, "volume_averaged_prices");

  file.close();

  return Candles(instrument, comment, timeframe, times, opens, highs, lows,
      closes, volumes, volumeAveragedPrices);
}
