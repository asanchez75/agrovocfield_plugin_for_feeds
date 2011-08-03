<?php
/**
 * @file
 * Enable the user of feeds to map taxonomy terms from the feed for the
 * current node to agrovocfield CCK fields.
 */

/**
 * Implementation of hook_feeds_node_processor_targets_alter().
 *
 * @see FeedsNodeProcessor::getMappingTargets().
 */
function agrovocfield_feeds_node_processor_targets_alter(&$targets, $content_type) {
  $info = content_types($content_type);
  $fields = array();
  if (isset($info['fields']) && count($info['fields'])) {
    foreach ($info['fields'] as $field_name => $field) {
      if(in_array($field['type'], array('agrovocfield'))) {
        $name = isset($field['widget']['label']) ? $field['widget']['label'] : $field_name;
        $targets[$field_name] = array(
          'name' => $name,
          'callback' => 'agrovocfield_feeds_set_target',
          'description' => t('The CCK %name field of the node (!type).', array('%name' => $name, '!type' => $field['type'])),
        );
      }
    }
  }
}

/**
 * Callback for mapping. Here is where the actual mapping happens.
 *
 * @param $node
 *   Reference to the node object we are working on.
 *
 * @param $vid
 *   The selected agrovocfield CCK field.
 *
 * @param $terms
 *   Given terms as array. If a string is used, it is converted to an array
 *   using <code>taxonomy_terms_parse_string($terms)->tids</code>.
 *
 * @see taxonomy_terms_parse_string().
 *
 */
function agrovocfield_feeds_set_target(&$node, $field_name, $terms) {
	static $fields = array();
  $field = content_fields($field_name, $node->type);
  // Return if there are no or empty terms.
  if (!is_array($terms) || empty($terms)) {
    return;
  }
  $typed_input = implode(',', $terms);
  $existing_tids = array();
  $new_tids = array();
  $extracted_ids  = agrovocfield_autocomplete_tags_get_tids($typed_input, $field['vid'], $parent = 0);
  $existing_tids = is_array($extracted_ids['existing_tids']) ? $extracted_ids['existing_tids'] : array();
  if (is_array($extracted_ids['non_existing_terms'])) {
    $new_tids = agrovocfield_autocomplete_insert_tags($extracted_ids['non_existing_terms']);          
  }
  $array_values = array_merge($existing_tids, $new_tids);
  if(!is_array($node->$field_name)) $node->$field_name = array();
  // Get first key ('value') from $field['columns']
  $key = array_keys($field['columns']); 
  $node->$field_name = content_transpose_array_rows_cols(array($key[0] => $array_values));
}


