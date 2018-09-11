#include <sick_microscan3_ros_driver/data_processing/ParseMeasurementData.h>

namespace sick {
namespace data_processing {

ParseMeasurementData::ParseMeasurementData()
{
  m_reader_ptr = boost::make_shared<sick::data_processing::ReadWriteHelper>();

}

datastructure::MeasurementData ParseMeasurementData::parseUDPSequence(datastructure::PacketBuffer buffer, datastructure::Data &data)
{
  std::cout << "Beginn Parsing Header" << std::endl;

  //TODO sanity checks
  if ( data.getDataHeaderPtr()->getMeasurementDataBlockOffset() == 0 && data.getDataHeaderPtr()->getMeasurementDataBlockSize() == 0) {
    return datastructure::MeasurementData();
  }

  const BYTE* data_ptr(buffer.getBuffer().data() + data.getDataHeaderPtr()->getMeasurementDataBlockOffset());

  datastructure::MeasurementData measurement_data;

  measurement_data.setNumberOfBeams(m_reader_ptr->readUINT32LittleEndian(data_ptr,0));
  std::cout << "NumberOfBeams: " << measurement_data.getNumberOfBeams() << std::endl;

  float angle = data.getDerivedValuesPtr()->getStartAngle();
  std::cout << angle << std::endl;
  float angle_delta = data.getDerivedValuesPtr()->getAngularBeamResolution();

  for (int i = 0; i < measurement_data.getNumberOfBeams(); i++)
  {
    INT16 distance = m_reader_ptr->readUINT16LittleEndian(data_ptr, (4 + i * 4));

    UINT8 reflectivity = m_reader_ptr->readUINT8LittleEndian(data_ptr, (6 + i *4) );

    UINT8 status = m_reader_ptr->readUINT8LittleEndian(data_ptr, (7+i*4));


    //TODO
    bool valid = status & (0x01 << 0);

    bool infinite = status & (0x01 << 1);

    bool glare = status & (0x01 << 2);

    bool reflector = status & (0x01 << 3);

    bool contamination = status & (0x01 << 4);

    bool contamination_warning = status & (0x01 << 5);

    measurement_data.addScanPoint(sick::datastructure::ScanPoint(angle, distance, reflectivity, valid, infinite, glare, reflector, contamination, contamination_warning));

    angle += angle_delta;

  }

  std::cout  << "measurement data size: " << measurement_data.getScanPointsVector().size() << std::endl;

//  std::cout  << "measurement data at 10: " << measurement_data.getScanPoints().at(714).getDistance() << std::endl;
//  std::cout  << "measurement data at 10: " << measurement_data.getScanPoints().at(714).getAngle() << std::endl;
//  std::cout  << "measurement data at 10: " << measurement_data.getScanPoints().at(714).getValidBit() << std::endl;

  return measurement_data;
}

}
}
