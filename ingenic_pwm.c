#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define PWM_NUM          8
#define PWM_QUERY_STATUS 0x200
#define PWM_CONFIG       0x001
#define PWM_CONFIG_DUTY  0x002
#define PWM_ENABLE       0x010
#define PWM_DISABLE      0x100

struct pwm_ioctl_t {
    int index;
    int duty;
    int period;
    int polarity;
    int enabled;
};

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("  --channel=<channel_num>\tSpecify PWM channel number [0-%d]\n", PWM_NUM-1);
    printf("  --query\t\t\tQuery the state of the specified PWM channel\n");
    printf("  --enable\t\t\tEnable the specified PWM channel\n");
    printf("  --disable\t\t\tDisable the specified PWM channel\n");
    printf("  --polarity=<0|1>\t\tSet polarity\n");
    printf("  --duty=<duty_ns>\t\tSet duty in nanoseconds\n");
    printf("  --period=<period_ns>\t\tSet period in nanoseconds\n");
}

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

int main(int argc, char *argv[]) {
    int channel = -1;
    int polarity = -1;
    int duty_ns = -1;
    int period_ns = -1;
    int enable = 0;
    int disable = 0;
    int query = 0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--channel=", 10) == 0) {
            channel = atoi(argv[i] + 10);
        } else if (strncmp(argv[i], "--polarity=", 11) == 0) {
            polarity = atoi(argv[i] + 11);
        } else if (strcmp(argv[i], "--enable") == 0) {
            enable = 1;
        } else if (strcmp(argv[i], "--disable") == 0) {
            disable = 1;
        } else if (strncmp(argv[i], "--duty=", 7) == 0) {
            duty_ns = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "--period=", 9) == 0) {
            period_ns = atoi(argv[i] + 9);
        } else if (strcmp(argv[i], "--query") == 0) {
            query = 1;
        } else {
            print_usage(argv[0]);
            return -1;
        }
    }

    if (channel < 0 || channel >= PWM_NUM) {
        printf("Error: Invalid PWM channel number.\n");
        return -1;
    }

    int fd = open("/dev/pwm", O_RDWR);
    if (fd < 0) {
        perror("Error opening /dev/pwm, make sure pwm drivers are enabled");
        return -1;
    }

    struct pwm_ioctl_t pwm_data;
    pwm_data.index = channel;

    if (polarity != -1) {
        pwm_data.polarity = polarity;
        // Call ioctl to set polarity
        ioctl(fd, PWM_CONFIG, &pwm_data);
    }

    if (duty_ns != -1) {
        pwm_data.duty = duty_ns;
        // Call ioctl to set duty cycle
        ioctl(fd, PWM_CONFIG_DUTY, &pwm_data);
    }

    if (period_ns != -1) {
        pwm_data.period = period_ns;
        // Call ioctl to set period
        ioctl(fd, PWM_CONFIG, &pwm_data);
    }

    if (enable) {
        // Call ioctl to enable PWM
        ioctl(fd, PWM_ENABLE, channel);
    }

    if (disable) {
        // Call ioctl to disable PWM
        ioctl(fd, PWM_DISABLE, channel);
    }

    if (query) {
        query_pwm_status(fd, channel);
    }

    close(fd);
    return 0;
}
