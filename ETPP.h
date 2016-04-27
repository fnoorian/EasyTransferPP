/******************************************************************
* EasyTransferPP Arduino Library version 0.9.1
*
* By Farzad Noorian <farzad.noorian@gmail.com>
* and Kwajo Menash <kwajom@gmail.com>
*
* Based on EasyTransfer protocol
* <http://www.billporter.info/easytransfer-arduino-library/>
* By Mathieu Alorent and Bill Porter <www.billporter.info>
*
* The EasyTransfer protocol is:
*   0x06, 0x85, size of payload (uint8_t), Payload, Checksum (in uint8_t)
* The checksum is computed by xoring "size of payload" with each byte of payload
*
* This is a complete and clean rewrite of the EasyTransfer protocol
* using advanced C++ techniques to facilitate easy porting to different
* communication tools, and is distributed under
* Apache License, Version 2.0 (the "License");
*
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License. 
*/

template<typename STREAM_TYPE, typename RECEIVE_TYPE = char>
class EasyTransferPP {
  
protected:  
  enum ReceiveState_t {
    STATE_HEADER_1,
    STATE_HEADER_2,
    STATE_LEN,
    STATE_BODY,
    STATE_CS
  } m_rx_state;
  
  uint8_t m_rx_size;
  uint8_t m_rx_counter;
  uint8_t m_rx_CS;
  
  STREAM_TYPE &m_stream;
  uint8_t *m_data_pointer;
  uint8_t m_data_size;

  uint8_t m_internal_buffer[sizeof(RECEIVE_TYPE)];
    
public:
  const int HEADER_BYTE_1 = 0x06;
  const int HEADER_BYTE_2 = 0x85;

  EasyTransferPP(STREAM_TYPE &stream_object) :
      m_stream(stream_object),
      m_rx_state(STATE_HEADER_1) 
  {
    setReceiveBuffer();
  } 

  template<class T>
  EasyTransferPP(STREAM_TYPE &stream_object, T &receive_buffer) :
      m_stream(stream_object),
      m_rx_state(STATE_HEADER_1) 
  {
    setReceiveBuffer(receive_buffer);
  }

  // set receive buffer to internal buffer
  void setReceiveBuffer() 
  {
    setReceiveBuffer(m_internal_buffer, sizeof(RECEIVE_TYPE));
  }
    
  // set receive buffer with template
  template<class T>
  void setReceiveBuffer(T &data_struct)
  {
    setReceiveBuffer( (uint8_t *) (&data_struct),  sizeof(T));
  }
  
  void setReceiveBuffer(uint8_t * data_pointer, uint8_t size)
  {
    m_data_pointer = data_pointer;
    m_data_size = size;
  }

  const RECEIVE_TYPE & receiveBuffer() const {
    return (const RECEIVE_TYPE &)(*((RECEIVE_TYPE *)m_internal_buffer));
  }

  // Begin transmission to a slave, and send the data
  template<class T>
  void sendTo(uint8_t slave_address, const T  &data_struct){
    sendTo(slave_address, (uint8_t *) (&data_struct), sizeof(T));
  }

  void sendTo(uint8_t slave_address, uint8_t const * const address, uint8_t size) {

    m_stream.beginTransmission(slave_address);

    send(address, size);

    m_stream.endTransmission();
  }  
  
  // send data with template
  template<class T>
  void send(const T  &data_struct){
    send((uint8_t *) (&data_struct), sizeof(T));
  }

  // send data
  void send(uint8_t const * const data_pointer, uint8_t size) {

    const int buffer_size = 2 + 1 + size + 1;
    uint8_t buffer[buffer_size];
    
    buffer[0] = HEADER_BYTE_1;
    buffer[1] = HEADER_BYTE_2;
    buffer[2] = size;

    uint8_t CS = size;
    for(int i = 0; i<size; i++){
      CS ^= data_pointer[i];
      buffer[i + 3] = data_pointer[i];
    }
    buffer[buffer_size - 1] = CS;

    m_stream.write(buffer, buffer_size);
  }

  // Request data with template
  template<class T>
  bool requestFrom(uint8_t slave_address)
  {
    return requestFrom(slave_address, m_internal_buffer, sizeof(RECEIVE_TYPE));
  }
  
  template<class T>
  bool requestFrom(uint8_t slave_address, T &data_struct)
  {
    return requestFrom(slave_address, (uint8_t *) (&data_struct), sizeof(T));
  }
  
  // Request data from a slave, and receive it
  bool requestFrom(uint8_t slave_address, uint8_t *data_pointer, uint8_t data_size)
  {
    setReceiveBuffer(data_pointer, data_size);

    int transfer_size = 2 + 1 + m_data_size + 1;
    m_stream.requestFrom((int) slave_address, transfer_size); // casting to integer to supress a warning

    return receive();
  }   

  // Receive data from the stream
  bool receive(int bytenum = -1) {

    while (m_stream.available() >= 1) {
      
      uint8_t rx = m_stream.read();

      if (m_data_pointer == NULL) {
          continue;
      }    
    
      switch(m_rx_state) {

        case STATE_HEADER_1:
          if (rx == HEADER_BYTE_1) m_rx_state = STATE_HEADER_2;
          break;
        
        case STATE_HEADER_2:
          if (rx == HEADER_BYTE_2) 
              m_rx_state = STATE_LEN;
          else 
              m_rx_state = STATE_HEADER_1;
          
          break;

        case STATE_LEN:
          m_rx_size = rx;
          m_rx_counter = 0;
          m_rx_CS = rx;
          m_rx_state = STATE_BODY;
          break;
      
        case STATE_BODY:
          if (m_rx_counter < m_data_size) {
              m_data_pointer[m_rx_counter] = rx;
          }
        
          m_rx_CS ^= rx;
          m_rx_counter++;
        
          if (m_rx_counter == m_rx_size) m_rx_state = STATE_CS;
        
          break;
      
        case STATE_CS:

          m_rx_state = STATE_HEADER_1;
      
          if (rx == m_rx_CS) {
            return true;
          }
       
          break;
      }
    }

    return false;
  }
};
