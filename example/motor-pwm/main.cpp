#include <stdio.h>

#include <pico/stdlib.h>
#include <hardware/pwm.h>

#define MG90S_PWM_PIN 16
#define MG996R_PWM_PIN 17

#define CLOCK 125 // per microsecond

uint64_t MG90S_FREQ = 50;
uint64_t MG996R_FREQ = 50;

int main()
{
	stdio_init_all();

	gpio_set_function(MG90S_PWM_PIN, GPIO_FUNC_PWM);
	gpio_set_function(MG996R_PWM_PIN, GPIO_FUNC_PWM);

	// Initialize PWM
	uint slice0 = pwm_gpio_to_slice_num(MG90S_PWM_PIN);
	uint slice1 = pwm_gpio_to_slice_num(MG996R_PWM_PIN);

	uint channel0 = pwm_gpio_to_channel(MG90S_PWM_PIN);
	uint channel1 = pwm_gpio_to_channel(MG996R_PWM_PIN);

	pwm_set_clkdiv(slice0, 256.0f);
	pwm_set_clkdiv(slice1, 256.0f);
	pwm_set_wrap(slice0, 9804);
	pwm_set_wrap(slice1, 9804);

	pwm_set_enabled(slice0, true);
	pwm_set_enabled(slice1, true);
	while (true)
	{
		pwm_set_chan_level(slice0, channel0, 294); /// Setting the duty period (0.6 ms)
		pwm_set_chan_level(slice1, channel1, 294); /// Setting the duty period (0.6 ms)
		sleep_ms(1000);
		// pwm_set_chan_level(slice0, channel0, 735); /// Setting the duty period (1.5 ms)
		// pwm_set_chan_level(slice1, channel1, 735); /// Setting the duty period (1.5 ms)
		// sleep_ms(1000);
		pwm_set_chan_level(slice0, channel0, 1176); /// Setting the duty period (2.4 ms)
		pwm_set_chan_level(slice1, channel1, 1176); /// Setting the duty period (2.4 ms)
		sleep_ms(1000);
	}
}