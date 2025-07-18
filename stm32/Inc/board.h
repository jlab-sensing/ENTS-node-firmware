
/**
 * @brief Shared code for programs
 *
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2025-07-18
 */

#ifndef INC_BOARD_H_
#define INC_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void);

/**
* @brief  This function is executed in case of error occurrence.
*
* Turns on a solid status LED to give visual indicator to user. Enters an
* infinite loop to halt program execution.
*
* @retval None
*/
void Error_Handler(void);

/**
 * @brief Waits for desktop serial connection to be established.
 *
 * Interally its just a NOP loop that waits for the serial monitor to be ready
 * on the desktop. Useful for getting continuous flow of unit tests.
 */
void WaitForSerial(void);

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line);
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* INC_BOARD_H_ */
