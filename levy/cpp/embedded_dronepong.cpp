#include <stdio.h>

/******************* RISP NETWORK CODE ***********************/

#define NUM_NEURONS (11)
#define NUM_INPUT_NEURONS (2)
#define NUM_OUTPUT_NEURONS (2)
#define NUM_SYNAPSES (20)
#define MAX_NUM_TIMESTEPS (51)
#define MAX_OUTGOING (4)
#define MIN_POTENTIAL (-10.000000)
#define SPIKE_VALUE_FACTOR (10.000000)

/* Synapse struct */
typedef struct {
    unsigned int to;    /* Index of to neuron */
    unsigned int delay; /* Synapse delay value */
    double weight;       /* Synapse weight value */
} Synapse;

/* Neuron struct */
typedef struct {
    unsigned char leak;                         /* Leak value (1 for full leak and 0 for no leak) */
    unsigned char check;                        /* Whether or not we have checked if this neuron fires */
    unsigned int num_outgoing;                  /* Number of outgoing synapses for this neuron */
    unsigned int fire_count;                    /* Number of fires */
    int last_fire;                              /* Last firing time */
    double charge;                               /* Charge value */
    double threshold;                            /* Threshold value */
    Synapse outgoing[MAX_OUTGOING];             /* Outgoing synapses */
    unsigned int fire_times[MAX_NUM_TIMESTEPS]; /* Firing times */
} Neuron;

/* Charge change event struct (essentially just a pair) */
typedef struct {
    unsigned int neuron_ind; /* Index of neuron to change the charge for */
    double charge_change;     /* Value to change charge by */
} Charge_Change_Event;

const unsigned int INPUT_IND_TO_NEURON_IND[NUM_INPUT_NEURONS] = {0, 1};
const unsigned int OUTPUT_IND_TO_NEURON_IND[NUM_OUTPUT_NEURONS] = {2, 3};

unsigned int event_count[MAX_NUM_TIMESTEPS] = {0};                   /* Number of charge change events for each timestep */
unsigned int cur_charge_changes_ind = 0;                             /* Index of charge changes array that represents which array of charge change events corresponds to the upcoming timestep */
Charge_Change_Event charge_changes[MAX_NUM_TIMESTEPS][NUM_SYNAPSES]; /* Charge changes keyed on timestep and charge change event index */
Neuron neurons[NUM_NEURONS] = { {0, 0, 2, 0, -1, 0, 0.000000, {{5,3,2.000000}, {2,3,3.000000}}, {0}},
                                {0, 0, 1, 0, -1, 0, 9.000000, {{3,2,-2.000000}}, {0}},
                                {0, 0, 1, 0, -1, 0, 9.000000, {{9,1,-5.000000}}, {0}},
                                {0, 0, 4, 0, -1, 0, 3.000000, {{1,2,10.000000}, {10,5,-1.000000}, {2,1,-4.000000}, {4,5,-2.000000}}, {0}},
                                {0, 0, 2, 0, -1, 0, 8.000000, {{4,2,3.000000}, {7,2,4.000000}}, {0}},
                                {0, 0, 4, 0, -1, 0, 8.000000, {{1,5,-3.000000}, {6,2,2.000000}, {0,3,-6.000000}, {3,1,10.000000}}, {0}},
                                {0, 0, 1, 0, -1, 0, 1.000000, {{1,4,7.000000}}, {0}},
                                {0, 0, 1, 0, -1, 0, 6.000000, {{8,1,-1.000000}}, {0}},
                                {0, 0, 2, 0, -1, 0, 8.000000, {{0,2,-1.000000}, {1,2,-2.000000}}, {0}},
                                {0, 0, 1, 0, -1, 0, 5.000000, {{4,1,1.000000}}, {0}},
                                {0, 0, 1, 0, -1, 0, 3.000000, {{5,5,0.000000}}, {0}} };

/* This function will apply a spike of potential value value to the input neuron with an input neuron zero-based index of input_ind at time time relative to the current timestep of the neuroprocessor. */
void apply_spike(unsigned int input_ind, unsigned int time, double value) {
    unsigned int target_charge_changes_ind;

    /* Ensure input neuron index is not out of bounds */
    if (input_ind >= NUM_INPUT_NEURONS) {
        return;
    }

    /* Ensure time is not out of bounds */
    if (time >= MAX_NUM_TIMESTEPS) {
        return;
    }

    target_charge_changes_ind = (cur_charge_changes_ind + time) % MAX_NUM_TIMESTEPS;

    /* Schedule charge change at current timestep for neuron with given index */
    if(event_count[target_charge_changes_ind] < NUM_SYNAPSES) {
        charge_changes[target_charge_changes_ind][event_count[target_charge_changes_ind]].neuron_ind = INPUT_IND_TO_NEURON_IND[input_ind];
        charge_changes[target_charge_changes_ind][event_count[target_charge_changes_ind]].charge_change = value * SPIKE_VALUE_FACTOR;
        event_count[target_charge_changes_ind]++;
    }
}

/* This function will run the SNN for duration, the specified number of timesteps (many neuroprocessors only support discrete timesteps, such as RISP). */
void run(double duration) {
    unsigned int time;
    unsigned int i;
    unsigned int j;
    unsigned int run_time;
    unsigned int cur_neuron_ind;
    unsigned int to_time;

    /* Clear tracking info on all neurons */
    for (i = 0; i < NUM_NEURONS; i++) {
        neurons[i].last_fire = -1;
        neurons[i].fire_count = 0;
    }

    /* Ensure run_time is not negative */
    if (duration-1 < 0) {
        return;
    }

    run_time = (unsigned int)(duration-1);
    
    for (time = 0; time <= run_time; time++) {

        /* Reset minimum charge before the next run */
        for (i = 0; i < event_count[cur_charge_changes_ind]; i++) {
            cur_neuron_ind = charge_changes[cur_charge_changes_ind][i].neuron_ind;
            if (neurons[cur_neuron_ind].charge < MIN_POTENTIAL) {
                neurons[cur_neuron_ind].charge = MIN_POTENTIAL;
            }
        }

        /* Collect charges */
        for (i = 0; i < event_count[cur_charge_changes_ind]; i++) {
            cur_neuron_ind = charge_changes[cur_charge_changes_ind][i].neuron_ind;
            neurons[cur_neuron_ind].check = 1;
            neurons[cur_neuron_ind].charge += charge_changes[cur_charge_changes_ind][i].charge_change;
        }

        /* Determine if neuron fires */
        for (i = 0; i < event_count[cur_charge_changes_ind]; i++) {
            cur_neuron_ind = charge_changes[cur_charge_changes_ind][i].neuron_ind;

            if (neurons[cur_neuron_ind].check == 1) {
                /* Fire if neuron charge meets its threshold */
                if (neurons[cur_neuron_ind].charge >= neurons[cur_neuron_ind].threshold) {
                    for (j = 0; j < neurons[cur_neuron_ind].num_outgoing; j++) {
                        to_time = (cur_charge_changes_ind + neurons[cur_neuron_ind].outgoing[j].delay) % MAX_NUM_TIMESTEPS;
                        if (event_count[to_time] < NUM_SYNAPSES) {
                            charge_changes[to_time][event_count[to_time]].neuron_ind = neurons[cur_neuron_ind].outgoing[j].to;
                            charge_changes[to_time][event_count[to_time]].charge_change = neurons[cur_neuron_ind].outgoing[j].weight;
                            event_count[to_time]++;
                        }
                    }

                    neurons[cur_neuron_ind].fire_times[neurons[cur_neuron_ind].fire_count] = time;
                    neurons[cur_neuron_ind].last_fire = time;
                    neurons[cur_neuron_ind].fire_count++;
                    neurons[cur_neuron_ind].charge = 0;
                }

                neurons[cur_neuron_ind].check = 0;
            }
        }

        /* "Shift" (using ring buffer) extra spiking events up a timestep to progress to the next timestep */
        event_count[cur_charge_changes_ind] = 0;
        cur_charge_changes_ind = (cur_charge_changes_ind + 1) % MAX_NUM_TIMESTEPS;
    }

    /* Reset minimum charge before the next run */
    for (i = 0; i < NUM_NEURONS; i++) {
        if (neurons[i].charge < MIN_POTENTIAL) {
            neurons[i].charge = MIN_POTENTIAL;
        }
    }
}

/* This function will clear the SNN of all activity. It resets all neuron and synapse state. */
void clear_activity() {
    unsigned int i;

    /* Clear activity-related neuron state */
    for (i = 0; i < NUM_NEURONS; i++) {
        neurons[i].last_fire = -1;
        neurons[i].fire_count = 0;
        neurons[i].charge = 0;
    }

    /* Clear all event activity */
    for (i = 0; i < MAX_NUM_TIMESTEPS; i++) {
        event_count[i] = 0;
    }
}

/* This function will return the timestep of the output neuron with an output neuron zero-based index of output_ind. The returned timestep will only be for the most recent call of the run() function. */
double output_last_fire(unsigned int output_ind) {
    
    /* Ensure output index not out of bounds */
    if (output_ind >= NUM_OUTPUT_NEURONS) {
        return -1;
    }

    return (double)neurons[OUTPUT_IND_TO_NEURON_IND[output_ind]].last_fire;
}

/* This function will return the number of neuronal fires for the output neuron with an output neuron zero-based index of output_ind. The returned fire count will only be for the most recent call of the run() function. */
unsigned int output_count(unsigned int output_ind) {
    
    /* Ensure output index not out of bounds */
    if (output_ind >= NUM_OUTPUT_NEURONS) {
        return 0;
    }

    return neurons[OUTPUT_IND_TO_NEURON_IND[output_ind]].fire_count;
}


/******************* SPIKE ENCODING CODE ***********************/

#define NUM_ENCODERS (2)
#define NUM_UNIQUE_ENCODERS (1)
#define TOT_MAX_ENCODED_SPIKES (100)

typedef struct {
    int id;       /* Represents the input id of the destination neuron */
    double time;  /* Represents the timing of when the spike should arrive */
    double value; /* Represents the charge to accumulate */
} Spike;

const double ENCODER_ROUNDING_THRESHOLD = 0.00000001;
const double ENCODER_DMINS[NUM_ENCODERS] = {-2000.000000,-0.500000};
const double ENCODER_DMAXS[NUM_ENCODERS] = {2000.000000,0.500000};
const unsigned int ENCODER_STARTING_NEURONS[NUM_ENCODERS] = {0,1};
const double ENCODER_SPIKE_MINS[NUM_UNIQUE_ENCODERS] = {1.000000};
const double ENCODER_SPIKE_MAXS[NUM_UNIQUE_ENCODERS] = {1.000000};
const int ENCODER_EFFECTIVE_NBINS[NUM_UNIQUE_ENCODERS] = {1};

Spike encoded_spikes[TOT_MAX_ENCODED_SPIKES]; /* Array of all spikes that have been encoded by the spike encoder */
unsigned int num_encoded_spikes = 0;          /* Size of encoded_spikes array */
double encoder_vals[NUM_ENCODERS] = {0};       /* Array of input values that are to be encoded into spikes during spike encoding */
double encoder_actual_intervals[NUM_UNIQUE_ENCODERS] = {50.000000};
double encoder_actual_max_spikes[NUM_UNIQUE_ENCODERS] = {50.000000};

/* This function will append a spike with the given id, time, and value to the encoded_spikes array. It is a helper function and will normally not be needed by the user. */
void append_encoded_spike(int id, double time, double value) {
    if (num_encoded_spikes >= TOT_MAX_ENCODED_SPIKES) {
        return;
    }

    encoded_spikes[num_encoded_spikes].id = id;
    encoded_spikes[num_encoded_spikes].time = time;
    encoded_spikes[num_encoded_spikes].value = value;

    printf("    spike: id=%d time=%f value=%f\n", id, time, value);

    num_encoded_spikes++;
}

/* This function will clear the encoded_spikes array of all its spikes. */
void clear_encoded_spikes() {
    num_encoded_spikes = 0;
}

/* This function will encode the given value val into spikes that are appended to the global array encoded_spikes. min and max describe what should be the minimum and maximum values of the inputted value, and starting_neuron is the zero-based index of the first input neuron corresponding to this single unique spike encoder. */
void encode_0(double val, double min, double max, unsigned int starting_neuron) {
    unsigned char found_bounds_bin;
    int i;
    int bin;
    int intrabin_bin;
    int pulses;
    int spike_id;
    double spike_time;
    double spike_value;
    double binv;
    double remainder;
    double intrabin_remainder;
    double lower_bound;
    double upper_bound;
    double intrabin_value;
    double interval;
    double pulses_value;
    double power;

    /* Deal with encoder_val outside of min/max */
    if (val < min) {
        val = min;
    }

    if (val > max) {
        val = max;
    }
    
    /* Ignore min encoder_val by not appending any more spikes */
    if (val == min) {
        return;
    }

    remainder = 0;
    bin = 0;

    /* Map encoder_val to the proper bin(s) */
    (void)found_bounds_bin;
    (void)lower_bound;
    (void)upper_bound;
    if (min == max) {
        binv = (double) ENCODER_EFFECTIVE_NBINS[0];
    } else {
        binv = (val - min) / (max - min) * (double) ENCODER_EFFECTIVE_NBINS[0];
    }

    bin = (int) binv;
    remainder = binv - bin;

    /* Handle remainder that is within ENCODER_ROUNDING_THRESHOLD */
    if (remainder < ENCODER_ROUNDING_THRESHOLD && bin != 0) {
        bin--;
        remainder = 1.0;
    } else if (remainder > 1.0 - ENCODER_ROUNDING_THRESHOLD) {
        remainder = 1.0;
    }

    /* Convert bin and remainder to spike(s) based on intrabin/single-bin spike encoding technique */

    intrabin_value = remainder;
    intrabin_bin = bin;

    /* Set spike bin / input neuron index */
    spike_id = intrabin_bin + starting_neuron;

    /* Generate initial spike train followed by last spike */
    interval = encoder_actual_intervals[0] / (double) encoder_actual_max_spikes[0];

    /* Set pulses to be one minus the number of pulses that you're actually doing */
    pulses_value = intrabin_value * encoder_actual_max_spikes[0];
    pulses = (int) pulses_value;
    if (pulses == encoder_actual_max_spikes[0]) {
        pulses--;
    }

    /* Calculate the power of the last pulse */
    intrabin_remainder = pulses_value - (double) pulses;
    power = ENCODER_SPIKE_MINS[0] + intrabin_remainder * (ENCODER_SPIKE_MAXS[0] - ENCODER_SPIKE_MINS[0]);

    /* Turn the zero-indexed pulses into a one-indexed number */

    pulses++;

    /* Make the initial chain of pulses */

    spike_value = ENCODER_SPIKE_MAXS[0];
    for (i = 0; i < pulses-1; i++) {
        spike_time = interval * (double) i;
        append_encoded_spike(spike_id, spike_time, spike_value);
    }

    /* Generate the last spike */

    if (pulses == 1 || intrabin_remainder > 0) {
        spike_value = power;
        spike_time = interval * (double) (pulses-1);
        append_encoded_spike(spike_id, spike_time, spike_value);
    }

}

/* This function will encode all values in the global array encoder_vals into spikes that are appended to the global array encoded_spikes. This function wraps either all encode_<i>() or timeseries_encode_<i> functions (both listed above) to perform spike encoding for each input value in the encoder_vals array. In other words, this function performs the job of the spike encoder array by using one spike encoder to encode each of the input values. The user should fill in the encoder_vals array to provide input values to the system. When using timeseries spike encoding, be sure to fill in both encoder_vals and encoder_vals_sizes. */
void encode() {
    encode_0(encoder_vals[0], ENCODER_DMINS[0], ENCODER_DMAXS[0], ENCODER_STARTING_NEURONS[0]);
    encode_0(encoder_vals[1], ENCODER_DMINS[1], ENCODER_DMAXS[1], ENCODER_STARTING_NEURONS[1]);

}


/******************* SPIKE DECODING CODE ***********************/

#define NUM_DECODERS (1)
#define NUM_UNIQUE_DECODERS (1)
#define NUM_OUTPUT_NEURONS (2)

const double DECODER_ROUNDING_THRESHOLD = 0.00000001;
const double DECODER_DMINS[NUM_DECODERS] = {0.000000};
const double DECODER_DMAXS[NUM_DECODERS] = {1.000000};
const unsigned int DECODER_STARTING_NEURONS[NUM_DECODERS] = {0};
const double DECODER_ACTUAL_DIVISORS[NUM_UNIQUE_DECODERS] = {50.000000};
const unsigned int DECODER_BINS[NUM_UNIQUE_DECODERS] = {2};

int decoder_counts[NUM_OUTPUT_NEURONS] = {0};   /* Array of number of times each output neuron fired */
double decoder_vals[NUM_DECODERS] = {0};         /* Array of output values decoded from the SNN */

int round_double(double x) {
    if (x < 0) {
        return (int)(x - 0.5);
    } else {
        return (int)(x + 0.5);
    }
}

double abs_double(double x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

/* This function will decode the firing patterns of output neurons starting at the output neuron zero-based index starting_neuron and return the decoded output value. Based on the type of spike decoder generated, this function will either use the global array decoder_times to get most recent output spike temporal information or decoder_counts to get output spike count information. min and max describe what should be the minimum and maximum values of the decoded output value. This function acts as a single unique spike decoder. */
double decode_0(double min, double max, unsigned int starting_neuron) {
    unsigned int i;
    unsigned int output_neuron_ind;
    double v;
    double t;
    double bin_vals[NUM_OUTPUT_NEURONS];

    /* Deal with bin outside of the number of output neurons possible */
    if (starting_neuron + DECODER_BINS[0] - 1 >= NUM_OUTPUT_NEURONS) {
        return min;
    }

    for (i = 0; i < (unsigned int) DECODER_BINS[0]; i++) {
        output_neuron_ind = starting_neuron + i;

        v = decoder_counts[output_neuron_ind];
        v /= DECODER_ACTUAL_DIVISORS[0];

        bin_vals[i] = v;
    }

    /* Do WTA or LTA */

    output_neuron_ind = 0;
    for (i = 1; i < DECODER_BINS[0]; i++) {
        if (bin_vals[i] > bin_vals[output_neuron_ind]) {
            output_neuron_ind = i;
        }
    }

    t = output_neuron_ind;
    t /= (double) (DECODER_BINS[0]-1);
    t *= (max - min);
    t += min;

    v = round_double(t);
    if (abs_double(v-t) < DECODER_ROUNDING_THRESHOLD) {
        t = v;
    }

    return t;
}

/* This function will decode all output spikes into output values. The output spike information used by this function is in the global arrays decoder_times and/or decoder_counts. This function wraps all decode_<i>() functions (listed above) to perform spike decoding for each output value in the decoder_vals array. In other words, this function performs the job of the spike decoder array by using one spike decoder to decode each of the output values. This function will set the global array decoder_vals to an array of all the decoded output values for the user to use as desired. */
void decode() {
    decoder_vals[0] = decode_0(DECODER_DMINS[0], DECODER_DMAXS[0], DECODER_STARTING_NEURONS[0]);
}


const unsigned int SIM_TIME = 50;

/* This function will encode the values provided by the user in the global array encoder_vals into spikes using encoder(), use apply_spike() to apply all encoded spikes to the SNN, run the SNN for duration timesteps using run(), collect output spike information about output firing counts (using output_count()) and times (using output_times()), and finally decode the output spike information into output values stored in the global array decoder_vals using decode(). Also, when using timeseries spike encoding, be sure to fill in both encoder_vals and encoder_vals_sizes. This function will not have a duration parameter if the input SNN contains a sim_time value; the sim_time value is instead automatically used. This is the most convenient wrapper function in this generate C API and allows the user to simulate the core components of a neuromorphic system with a single function call. */
void encode_run_decode() {
    unsigned int i;
    
    clear_encoded_spikes();
    encode();
    
    for (i = 0; i < num_encoded_spikes; i++) {
        apply_spike(encoded_spikes[i].id, encoded_spikes[i].time, encoded_spikes[i].value);
    }
    
    run(SIM_TIME);
    
    for (i = 0; i < NUM_OUTPUT_NEURONS; i++) {
        decoder_counts[i] = output_count(i);
    }

    printf("decoder_counts[0]=%d decoder_counts[1]=%d\n",
        decoder_counts[0], decoder_counts[1]);
    
    decode();

    printf("\n----------------------------------------\n");
}
