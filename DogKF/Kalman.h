/* 
 Based on: https://www.scribd.com/document/70053826/Filtering-Sensor-Data-with-a-Kalman-Filter-Interactive-Matter
 * Simple 1D Kalman Filter
 */

#ifndef _Kalman_h
#define _Kalman_h

class Kalman {
    private:
    double q;
    double r;
    double x;
    double p;
    double k;
    
    public:
    Kalman(double process_noise, double sensor_noise, double estimated_error, double intial_value) {
         /*
         For large noise reduction, you can try to start from:
         q = 0.125
         r = 32
         p = 1023 //"large enough to narrow down"
         e.g.
         myVar = Kalman(0.125,32,1023,0);
         */
        this->q = process_noise;
        this->r = sensor_noise;
        this->p = estimated_error;
        this->x = intial_value; //x will hold the iterated filtered value
    }
    
    double getFilteredValue(double measurement) {
        this->p = this->p + this->q;
        
        this->k = this->p / (this->p + this->r);
        this->x = this->x + this->k * (measurement - this->x);
        this->p = (1 - this->k) * this->p;
        
        return this->x;
    }
    
    void setParameters(double process_noise, double sensor_noise, double estimated_error) {
        this->q = process_noise;
        this->r = sensor_noise;
        this->p = estimated_error;
    }
    
    void setParameters(double process_noise, double sensor_noise) {
        this->q = process_noise;
        this->r = sensor_noise;
    }
    
    double getProcessNoise() {
        return this->q;
    }
    
    double getSensorNoise() {
        return this->r;
    }
    
    double getEstimatedError() {
        return this->p;
    }
};

#endif
