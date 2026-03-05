# ArchiManage

ArchiManage is a desktop management platform built with Qt/C++ for architecture and engineering teams.  
It centralizes core workflows: employee management, clients, partners, designs, projects, role-based access, analytics, and reporting.

## Features

- Authentication and role-based access control
- Employee management (CRUD, filters, search, statistics)
- Client management with loyalty and notification workflows
- Partner management with communication tools
- Design management with catalog and export options
- Project management with status tracking and task organization
- PDF and Excel export in multiple modules
- Data visualization with Qt Charts
- Optional Arduino serial integration

## Tech Stack

- C++17
- Qt (Widgets, SQL, Network, Charts, SerialPort, AxContainer)
- SQL database access via ODBC
- Windows desktop target

## Project Structure

- `main.cpp`: application bootstrap and login flow
- `mainwindow.*`: central navigation shell
- `*mainwindow.*`: module-specific UI and actions
- `connection.*`: database connection setup
- `int2.pro`: Qt project definition

## Requirements

- Qt 6.x (or compatible Qt setup)
- C++ compiler toolchain (MinGW/MSVC)
- Configured ODBC DSN and database credentials

## Build and Run

1. Open `int2.pro` in Qt Creator.
2. Configure a valid Qt kit (Qt + compiler).
3. Build and run.

## Environment Variables

Sensitive integrations are configured through environment variables:

- `OPENAI_API_KEY`
- `TWILIO_ACCOUNT_SID`
- `TWILIO_AUTH_TOKEN`
- `TWILIO_FROM_NUMBER`
- `SMTP_FROM`
- `SMTP_USERNAME`
- `SMTP_PASSWORD`

## Security Notes

- Do not commit API keys, SMTP passwords, or tokens.
- Keep `build/` artifacts and user-specific files out of version control.
- Rotate any credential that was previously exposed.

## Status

Active cleanup and stabilization are in progress for public release quality.
