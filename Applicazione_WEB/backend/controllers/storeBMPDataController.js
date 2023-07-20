const Sensor_data = require('../model/Sensor_data');
const BMP280Model = Sensor_data.BMP280;

const handleStoreBMPData = async (req, res) => {

    try {

        //create and store the new user
        const result = await BMP280Model.create({

            timestamp: req.body.timestamp,
            temperature: req.body.temperature,
            altitude: req.body.altitude,
            pressure: req.body.pressure,

        });

        res.status(200).json({ result });
    } catch (err) {
        res.status(500).json({ 'Error on insert new record in MongoDB. Message ': err.message });
    }
}

module.exports = { handleStoreBMPData };