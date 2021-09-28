/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Neil Enns. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#pragma once

#include <map>
#include <mbed.h>
#include <optional>

#include "ArduinoTypes.hpp"
#include "modules/MFModuleTypes.hpp"

class PinManager
{
public:
  /**
   * @brief Construct a new Pin Manager object
   * 
   * @param numberOfPins The maximum number of supported pins.
   */
  PinManager(uint8_t numberOfPins);

  /**
   * @brief Clears all registered pins from the list.
   * 
   */
  void ClearRegisteredPins();

  /**
   * @brief Checks to see if a pin is registered with the system.
   * 
   * @param pin The Arduino pin number to check.
   * @return true If the pin is registered.
   * @return false If the pin is not registered.
   */
  bool IsPinRegistered(ARDUINO_PIN pin);

  /**
   * @brief Registers a new pin with the specified module type.
   * 
   * @param pin The Arduino pin to register.
   * @param type The module type to associate with the pin.
   */
  void RegisterPin(ARDUINO_PIN pin, MFModuleType type);

  /**
   * @brief Maps an Arduino pin number to an STM32 pin.
   * 
   * @param arduinoPin The Arduino pin number to map.
   * @return std::optional<PinName> The STM32 pin, if mapped. If the pin isn't mapped returns std::nullopt.
   */
  static std::optional<PinName> MapArudinoPin(ARDUINO_PIN arduinoPin);

private:
  map<ARDUINO_PIN, MFModuleType> *_registeredPins;
};
