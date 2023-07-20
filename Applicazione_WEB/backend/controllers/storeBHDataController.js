const Sensor_data = require('../model/Sensor_data');
const BH1750Model = Sensor_data.BH1750;

const handleStoreBHData = async (req, res) => {
    
    try {

        //create and store the new user
        const result = await BH1750Model.create({

            timestamp: req.body.timestamp,
            lux: req.body.lux,

        });

        res.status(200).json({ result });
    } catch (err) {
        res.status(500).json({ 'Error on insert new record in MongoDB. Message ': err.message });
    }
}

module.exports = { handleStoreBHData };