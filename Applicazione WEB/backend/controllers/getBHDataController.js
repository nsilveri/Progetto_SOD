const Sensor_data = require('../model/Sensor_data');
const BH1750Model = Sensor_data.BH1750;

const handleGetBHData = async (req, res) => {
    

    try{
        //create and store the new user
        const result = await BH1750Model.find({}).sort({ timestamp: 1 });
        
        res.status(200).json(result);
    } catch (err) {
        res.status(500).json({ 'Error on get values from DB. Message ': err.message });
    }
}

module.exports = { handleGetBHData };