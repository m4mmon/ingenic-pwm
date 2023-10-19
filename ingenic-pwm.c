#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include "version.h"

// Constants defining the number of PWM channels and their respective IOCTL commands
#define PWM_NUM          8
#define PWM_QUERY_STATUS 0x200
#define PWM_CONFIG       0x001
#define PWM_CONFIG_DUTY  0x002
#define PWM_ENABLE       0x010
#define PWM_DISABLE      0x100

// Structure representing the PWM configuration
struct pwm_ioctl_t {
    int index;     // PWM channel index
    int duty;      // PWM duty cycle in nanoseconds
    int period;    // PWM period in nanoseconds
    int polarity;  // PWM polarity (0: Inversed, 1: Normal)
    int enabled;   // Flag indicating if PWM is enabled or not
};

// Function to display the usage information for the program
void print_usage(const char *prog_name) {
    printf("INGENIC PWM Control Version: %s\n", VERSION);
    printf("Usage: %s [options]\n\n", prog_name);
    printf("Options:\n");
    printf("  -c, --channel=<0-%d>            Specify PWM channel number\n", PWM_NUM-1);
    printf("  -q, --query                    Query channel state\n");
    printf("  -e, --enable                   Enable channel\n");
    printf("  -d, --disable                  Disable channel\n");
    printf("  -p, --polarity=<0|1>           Set polarity (0: Inversed, 1: Normal)\n");
    printf("  -D, --duty=<duty_ns>           Set duty cycle in ns\n");
    printf("  -P, --period=<period_ns>       Set period in ns\n");
    printf("  -r, --ramp=<value>             Ramp PWM (+value: Ramp up, -value: Ramp down)\n");
    printf("  -x, --max_duty=<max_duty_ns>   Set max duty for ramping\n");
    printf("  -n, --min_duty=<min_duty_ns>   Set min duty for ramping\n");
    printf("  -h, --help                     Display this help message\n");
}

// Function to query and display the current status of a given PWM channel
void query_pwm_status(int fd, int channel) {
    struct pwm_ioctl_t pwm_status;
    pwm_status.index = channel;
    if (ioctl(fd, PWM_QUERY_STATUS, &pwm_status) != -1) {
        printf("PWM Channel %d Status:\n", channel);
        printf("Enabled: %s\n", pwm_status.enabled ? "Enabled" : "Disabled");
        printf("Polarity: %s\n", pwm_status.polarity ? "Normal" : "Inversed");
        printf("Duty: %d ns\n", pwm_status.duty);
        printf("Period: %d ns\n", pwm_status.period);
    } else {
        perror("Error querying PWM status");
    }
}

// Function to ramp up or down the PWM duty cycle based on provided parameters
void ramp_pwm(int fd, int channel, int ramp_value, int max_duty, int min_duty, int period_ns) {
    int duty;
    struct pwm_ioctl_t pwm_data;
    pwm_data.index = channel;
    pwm_data.period = period_ns;  // Set the period

    if (ramp_value > 0) {  // Ramp up
        for (duty = min_duty; duty <= max_duty; duty += abs(ramp_value)) {
            pwm_data.duty = duty;
            ioctl(fd, PWM_CONFIG_DUTY, &pwm_data);
            usleep(50000);  // Introduce a 50 ms delay between each change
        }
    } else if (ramp_value < 0) {  // Ramp down
        for (duty = max_duty; duty >= min_duty; duty -= abs(ramp_value)) {
            pwm_data.duty = duty;
            ioctl(fd, PWM_CONFIG_DUTY, &pwm_data);
            usleep(50000);  // Introduce a 50 ms delay between each change
        }
    }
}

int main(int argc, char *argv[]) {
    // Initialize default values for the variables representing command-line options
    int channel = -1;
    int polarity = -1;
    int duty_ns = -1;
    int period_ns = -1;
    int enable = 0;
    int disable = 0;
    int query = 0;
    int ramp_value = 0;  // 0 means no ramping. Positive means ramp up, negative means ramp down.
    int max_duty = -1;
    int min_duty = -1;

    // Define the long-format options for the command-line arguments
    struct option long_options[] = {
        {"channel", required_argument, NULL, 'c'},
        {"query", no_argument, NULL, 'q'},
        {"enable", no_argument, NULL, 'e'},
        {"disable", no_argument, NULL, 'd'},
        {"polarity", required_argument, NULL, 'p'},
        {"duty", required_argument, NULL, 'D'},
        {"period", required_argument, NULL, 'P'},
        {"ramp", required_argument, NULL, 'r'},
        {"max_duty", required_argument, NULL, 'x'},
        {"min_duty", required_argument, NULL, 'n'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    // Display usage information if no arguments are provided
    if (argc == 1) {
        print_usage(argv[0]);
        return 0;
    }

    // Parse the command-line arguments
    int opt;
    while ((opt = getopt_long(argc, argv, "c:qedp:D:P:r:x:n:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                channel = atoi(optarg);
                break;
            case 'q':
                query = 1;
                break;
            case 'e':
                enable = 1;
                break;
            case 'd':
                disable = 1;
                break;
            case 'p':
                polarity = atoi(optarg);
                break;
            case 'D':
                duty_ns = atoi(optarg);
                break;
            case 'P':
                period_ns = atoi(optarg);
                break;
            case 'r':
                ramp_value = atoi(optarg);
                break;
            case 'x':
                max_duty = atoi(optarg);
                break;
            case 'n':
                min_duty = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }

    // Validate the provided PWM channel number
    if (channel < 0 || channel >= PWM_NUM) {
        printf("Error: Invalid PWM channel number.\n");
        return -1;
    }

    // Open the PWM device for reading and writing
    int fd = open("/dev/pwm", O_RDWR);
    if (fd < 0) {
        perror("Error opening /dev/pwm, make sure pwm drivers are enabled");
        return -1;
    }

    // Structure to hold the PWM configuration for the ioctl calls
    struct pwm_ioctl_t pwm_data = {0};  // Initialize to zero
    pwm_data.index = channel;

    // Set the PWM period if provided
    if (period_ns != -1) {
        pwm_data.period = period_ns;
        ioctl(fd, PWM_CONFIG, &pwm_data);
    }

    // Set the PWM duty cycle if provided
    if (duty_ns != -1) {
        pwm_data.duty = duty_ns;
        ioctl(fd, PWM_CONFIG_DUTY, &pwm_data);
    }

    // Set the PWM polarity if provided
    if (polarity != -1) {
        pwm_data.polarity = polarity;
        ioctl(fd, PWM_CONFIG, &pwm_data);
    }

    // Enable the PWM channel if the 'enable' flag is set
    if (enable) {
        ioctl(fd, PWM_ENABLE, channel);
    }

    // Disable the PWM channel if the 'disable' flag is set
    if (disable) {
        ioctl(fd, PWM_DISABLE, channel);
    }

    // Query the PWM status if the 'query' flag is set
    if (query) {
        query_pwm_status(fd, channel);
    }

    // Ramp the PWM duty cycle if the 'ramp_value' is not zero
    if (ramp_value) {
        if (period_ns != -1) {
            ramp_pwm(fd, channel, ramp_value, max_duty, min_duty, period_ns);
        } else {
            printf("Error: A valid period must be specified for ramping.\n");
            close(fd);
            return -1;
        }
    }

    // Close the PWM device file descriptor
    close(fd);
    return 0;
}
