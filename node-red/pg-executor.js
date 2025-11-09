#!/usr/bin/env node
const { Client } = require('pg');

const config = {
    host: 'postgres-iottu.postgres.database.azure.com',
    port: 5432,
    database: 'iottu_postgres_db',
    user: 'lcasql',
    password: 'Challenge!DD25',
    ssl: { rejectUnauthorized: false }
};

const query = process.argv.slice(2).join(' ');

if (!query.trim()) {
    console.error('❌ Erro: Query não fornecida');
    process.exit(1);
}

async function executeQuery() {
    const client = new Client(config);
    
    try {
        await client.connect();
        const result = await client.query(query.trim());
        console.log(`✅ OK: ${result.rowCount} linha(s)`);
        process.exit(0);
    } catch (err) {
        console.error(`❌ Erro: ${err.message}`);
        process.exit(1);
    } finally {
        await client.end();
    }
}

executeQuery();
