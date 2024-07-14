# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at

#   http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.



from dash import html, dcc
from dash import dash_table
from dash.dependencies import Input, Output, State
import psutil
from dash import dash_table
import random
import string
import yaml


import dash_bootstrap_components as dbc




###################################################################################
##       CORE FUNCTIONS
###################################################################################



def get_activities_from_yaml(category=None):
    file_path = '/opt/ecofloc/activities.yaml'
    try:
        with open(file_path, 'r') as file:
            data = yaml.safe_load(file)
        if category:
            # If a specific category is requested, return its elements
            return data.get(category, [])
        else:
            # Otherwise, return all categories
            categories = [{'category': key} for key in data.keys()]
            return categories
    except Exception as e:
        print("Failed to read or parse the YAML file:", str(e))
        return []




###################################################################################
##       PANEL LAYOUT 
###################################################################################

def system_monitor_layout():
    return html.Div([
        # Left side: System Monitor Layout
        html.Div([
            dcc.Input(
                id='filter-input',
                type='text',
                placeholder='Enter PID or Name...',
                style={'margin': '10px', 'width': '300px'}
            ),
            dash_table.DataTable(
                id='process-table',
                columns=[
                    {'name': 'PID', 'id': 'pid'},
                    {'name': 'Name', 'id': 'name'},
                    {'name': 'CPU Usage', 'id': 'cpu_usage', 'type': 'numeric'}
                ],
                data=[],
                page_size=1000,
                sort_action='native',
                sort_mode='multi',
                row_selectable='single',
                style_table={'height': '400px', 'overflowY': 'auto', 'overflowX': 'auto'}
            ),
            html.Button('Select Application', id='select-button', className="button", n_clicks=0),
            dcc.Store(id='stored-selected-process'),  # Global Store for selected process
            html.Div(id='selected-process-info'),
            dcc.Interval(
                id='interval-processes-table',
                interval=2*1000,  # in milliseconds
                n_intervals=0
            )
        ], style={'display': 'inline-block', 'width': '50%'}),

        # Right side: Data Lake activities
        html.Div([
            dcc.Input(
                id='word-filter-input',
                type='text',
                placeholder='Enter activity...',
                style={'margin': '10px', 'width': '300px'}
            ),
            dash_table.DataTable(
                id='word-table',
                columns=[{'name': 'Category', 'id': 'category'}],
                data=get_activities_from_yaml(),
                page_size=100,
                sort_action='native',
                sort_mode='multi',
                row_selectable='single',
                style_table={'height': '400px', 'overflowY': 'auto', 'overflowX': 'auto', 'backgroundColor': 'white'}
            ),
            html.Button('Select Activity', id='select-activity-button', className="button", n_clicks=0),  
            dcc.Store(id='stored-selected-activity'),  # Global Store for selected activity
            html.Div(id='selected-activity-info'),  # Display for selected activity info
            dcc.Interval(
                id='interval-activities-table',
                interval=2000,  # in milliseconds (2000ms = 2 seconds)
                n_intervals=0
            )
        ], style={'display': 'inline-block', 'width': '50%'})
    ], style={'display': 'flex', 'width': '100%'})


###################################################################################
##       CALLBACKS
###################################################################################



def register_system_monitor_callbacks(app):


    ##UPDATE THE PROCESSES TABLE
    @app.callback(
        Output('process-table', 'data'),
        [Input('interval-processes-table', 'n_intervals'),
         Input('filter-input', 'value')]
    )
    def update_table(n, filter_value):
        processes = []
        for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_percent']):
            process_data = {
                'pid': proc.info['pid'],
                'name': proc.info['name'],
                'cpu_usage': proc.info['cpu_percent']
                # 'memory_usage': proc.info['memory_percent'],
                # 'nic_usage': 'Placeholder',
                # 'disk_usage': 'Placeholder'
            }
            if filter_value:
                if str(proc.info['pid']).startswith(filter_value) or filter_value.lower() in proc.info['name'].lower():
                    processes.append(process_data)
            else:
                processes.append(process_data)
        return processes



    #UPDATE THE ACTIVITIES TABLE

    @app.callback(
        Output('word-table', 'data'),
        [Input('word-filter-input', 'value'),
        Input('interval-activities-table', 'n_intervals')]
    )
    def update_table(filter_value, n_intervals):
        # Fetch categories from the YAML file
        all_categories = get_activities_from_yaml()  
        if filter_value is None:
            # No filter applied, return all categories
            return all_categories
        else:
            # Filter categories based on the input in the filter text box
            filtered_categories = [category for category in all_categories if filter_value.lower() in category['category'].lower()]
            return filtered_categories





    #SELECT PROCESS BUTTON 
    @app.callback(
        Output('stored-selected-process', 'data'),
        Input('select-button', 'n_clicks'),
        State('process-table', 'selected_rows'),
        State('process-table', 'data')
    )
    def store_selected_process(n_clicks, selected_rows, data):
        if n_clicks > 0 and selected_rows is not None and selected_rows:
            selected_process = data[selected_rows[0]]
            return {'pid': selected_process['pid'], 'name': selected_process['name']}
        return {}

    @app.callback(
        Output('selected-process-info', 'children'),
        Input('stored-selected-process', 'data')
    )
    def update_output_div(selected_data):
        if selected_data:
            return f"Selected Process PID: {selected_data['pid']}, Name: {selected_data['name']}"
        return "Select a process!"


    #SELECT ACTIVITY BUTTON

    @app.callback(
    Output('stored-selected-activity', 'data'),
    Input('select-activity-button', 'n_clicks'),
    State('word-table', 'selected_rows'),
    State('word-table', 'data')
    )
    def store_selected_activity(n_clicks, selected_rows, data):
        if n_clicks > 0 and selected_rows is not None and selected_rows:
            selected_category = data[selected_rows[0]]
            # Fetch elements for the selected category from the YAML file
            category_elements = get_activities_from_yaml(selected_category['category'])
            return {'elements': category_elements}  # Store only the elements
        return {}

    @app.callback(
    Output('selected-activity-info', 'children'),
    Input('stored-selected-activity', 'data')
    )
    def update_activity_output_div(selected_data):
        if selected_data and 'elements' in selected_data:
            elements = ", ".join(selected_data['elements'])
            return f"Selected Processes: {elements}"
        return "Select a category to see the elements!"
