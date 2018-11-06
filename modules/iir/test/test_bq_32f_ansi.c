#include <string.h>
#include "unity.h"
#include "test_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portable.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_clk.h"
#include "soc/cpu.h"
#include "esp_log.h"

#include "dsls_tone_gen.h"
#include "dsls_d_gen.h"
#include "dsls_biquad_gen.h"
#include "dsls_biquad.h"

static const char *TAG = "dsls_biquad_32f_ansi";

float x[1024];
float y[1024];

TEST_CASE("dsls_biquad_32f_ansi functionality", "[dsls]")
{
    // In the test we generate filter with cutt off frequency 0.1
    // and then filtering 0.1 and 0.3 frequencis.
    // Result must be better then 24 dB
    int len = sizeof(x)/sizeof(float);

    dsls_tone_gen_f32(x, len, 1, 0.1, 0);
//    dsls_d_gen_f32(x, len, 0);
    float coeffs[5];
    float w1[2] = {0};
    float w2[2] = {0};
    dsls_biquad_gen_lpf_32f(coeffs, 0.1, 1, 1);
    dsls_biquad_32f_ansi(x, y, len, coeffs, w1);
    float pow_band = 0;
    for (int i=len/2 ; i< len ; i++)
    {
        pow_band += y[i]*y[i];
    }
    float pow_out_band = 0;
    dsls_tone_gen_f32(x, len, 1, 0.3, 0);
    dsls_biquad_32f_ansi(x, y, len, coeffs, w2);
    for (int i=len/2 ; i< len ; i++)
    {
        pow_out_band += y[i]*y[i];
    }
    pow_band= 2* pow_band/(float)len;
    pow_out_band= 2* pow_out_band/(float)len;
    float diff_db = -10*log10f(0.000000001 + pow_out_band/pow_band);
    ESP_LOGI(TAG, "Power: pass =%f, stop= %f, diff = %f dB", pow_band, pow_out_band, diff_db);

    if (diff_db < 24)
    {
        ESP_LOGE(TAG, "Attenuation for LPF must be not less then 24! Now it is: %f", diff_db);
        TEST_ASSERT_MESSAGE (false, "LPF attenuation is less then expected");
    }
}

// TEST_CASE("partition parameters", "[dsl][ignore]")
// {
//     size_t size_before = xPortGetFreeHeapSize();
//     size_t size_after = xPortGetFreeHeapSize();

//     ptrdiff_t stack_diff = size_before - size_after; 
//     stack_diff = abs(stack_diff);
//     if (stack_diff > 8) TEST_ASSERT_EQUAL(0, stack_diff);
// }