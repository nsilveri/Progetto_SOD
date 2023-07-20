const express = require('express');
const router = express.Router();
const storeBHDataController = require('../controllers/storeBHDataController');

router.post('/', storeBHDataController.handleStoreBHData);

module.exports = router;