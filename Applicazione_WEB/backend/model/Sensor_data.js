const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const BMP280_schema = new Schema({
    timestamp: {
        type: Date,
        required: true
    },
    temperature: {
        type: String,
        required: true
    },
    pressure: {
        type: String,
        required: true
    },
    altitude: {
        type: String,
        required: true
    },
    refreshToken: String
});

const BH1750_schema = new Schema({
    timestamp: {
        type: Date,
        required: true
    },
    lux: {
        type: String,
        required: true
    },
    refreshToken: String
});

module.exports ={
    BMP280: mongoose.model('BMP280', BMP280_schema),
    BH1750: mongoose.model('BH1750', BH1750_schema),
} 